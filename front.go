package main

import (
    "bufio"
    "bytes"
    "fmt"
    "io"
    "os"
    "os/exec"
    "strings"
    "time"
)

func main() {
    childPath := "./calculator"
    if len(os.Args) > 1 {
        childPath = os.Args[1]
    }

    cmd := exec.Command(childPath)
    stdin, err := cmd.StdinPipe()
    if err != nil {
        fmt.Fprintf(os.Stderr, "failed to create stdin pipe: %v\n", err)
        os.Exit(1)
    }

    stdout, err := cmd.StdoutPipe()
    if err != nil {
        fmt.Fprintf(os.Stderr, "failed to create stdout pipe: %v\n", err)
        os.Exit(1)
    }

    stderrPipe, err := cmd.StderrPipe()
    if err != nil {
        fmt.Fprintf(os.Stderr, "failed to create stderr pipe: %v\n", err)
        os.Exit(1)
    }

    stderrBuf := &bytes.Buffer{}
    go func() {
        _, _ = io.Copy(stderrBuf, stderrPipe)
    }()

    if err := cmd.Start(); err != nil {
        fmt.Fprintf(os.Stderr, "unable to start child process %q: %v\n", childPath, err)
        os.Exit(1)
    }

    defer func() {
        _ = stdin.Close()
        if cmd.Process != nil {
            _ = cmd.Process.Kill()
        }
        _ = cmd.Wait()
    }()

    stdinWriter := bufio.NewWriter(stdin)
    stdoutReader := bufio.NewReader(stdout)
    userScanner := bufio.NewScanner(os.Stdin)

    for {
        fmt.Print("> ")
        if !userScanner.Scan() {
            fmt.Println()
            break
        }

        line := strings.TrimSpace(userScanner.Text())
        if line == "" {
            continue
        }

        if line == "exit" {
            if err := sendQuit(stdinWriter); err != nil {
                fmt.Fprintf(os.Stderr, "failed to send quit to child: %v\n", err)
            }
            break
        }

        if err := sendLine(stdinWriter, line); err != nil {
            fmt.Fprintf(os.Stderr, "failed to write to child stdin: %v\n", err)
            break
        }

        answer, err := stdoutReader.ReadString('\n')
        if err != nil {
            handleChildReadError(err, stderrBuf)
            break
        }

        fmt.Print(strings.TrimRight(answer, "\r\n") + "\n")
    }

    if userScanner.Err() != nil {
        fmt.Fprintf(os.Stderr, "input error: %v\n", userScanner.Err())
    }
}

func sendLine(writer *bufio.Writer, line string) error {
    if _, err := writer.WriteString(line + "\n"); err != nil {
        return err
    }
    return writer.Flush()
}

func sendQuit(writer *bufio.Writer) error {
    if _, err := writer.WriteString("quit\n"); err != nil {
        return err
    }
    if err := writer.Flush(); err != nil {
        return err
    }
    time.Sleep(50 * time.Millisecond)
    return nil
}

func handleChildReadError(err error, stderrBuf *bytes.Buffer) {
    if err == io.EOF {
        fmt.Fprintf(os.Stderr, "child process closed its output\n")
    } else {
        fmt.Fprintf(os.Stderr, "failed to read child response: %v\n", err)
    }
    if stderrStr := strings.TrimSpace(stderrBuf.String()); stderrStr != "" {
        fmt.Fprintf(os.Stderr, "child stderr: %s\n", stderrStr)
    }
}

