
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LINE_SIZE 1024
#define STACK_CAPACITY 256

static double stack[STACK_CAPACITY];
static int sp = 0;

static int push(double value)
{
    if (sp >= STACK_CAPACITY) {
        return -1;
    }
    stack[sp++] = value;
    return 0;
}

static int pop(double *value)
{
    if (sp <= 0) {
        return -1;
    }
    *value = stack[--sp];
    return 0;
}

static void process_line(const char *line)
{
    char buf[LINE_SIZE + 2];
    char *token;
    char *endptr;

    sp = 0;
    strncpy(buf, line, LINE_SIZE + 1);
    buf[LINE_SIZE + 1] = '\0';

    token = strtok(buf, " \t\r\n");
    if (token == NULL) {
        printf("ERROR: пустое выражение\n");
        fflush(stdout);
        return;
    }

    while (token != NULL) {
        double value;

        value = strtod(token, &endptr);
        if (endptr != token && *endptr == '\0') {
            if (push(value) != 0) {
                printf("ERROR: переполнение стека\n");
                fflush(stdout);
                return;
            }
        } else {
            size_t len = strlen(token);
            if (len == 1 && (token[0] == '+' || token[0] == '-' ||
                             token[0] == '*' || token[0] == '/')) {
                double b, a;
                char op = token[0];

                if (pop(&b) != 0 || pop(&a) != 0) {
                    printf("ERROR: недостаточно операндов для '%c'\n", op);
                    fflush(stdout);
                    return;
                }

                switch (op) {
                    case '+':
                        value = a + b;
                        break;
                    case '-':
                        value = a - b;
                        break;
                    case '*':
                        value = a * b;
                        break;
                    case '/':
                        if (b == 0.0) {
                            printf("ERROR: деление на ноль\n");
                            fflush(stdout);
                            return;
                        }
                        value = a / b;
                        break;
                    default:
                        value = 0.0;
                        break;
                }

                if (push(value) != 0) {
                    printf("ERROR: переполнение стека\n");
                    fflush(stdout);
                    return;
                }
            } else {
                printf("ERROR: неизвестный токен '%s'\n", token);
                fflush(stdout);
                return;
            }
        }

        token = strtok(NULL, " \t\r\n");
    }

    if (sp == 1) {
        printf("%.10g\n", stack[0]);
        fflush(stdout);
    } else if (sp == 0) {
        printf("ERROR: пустое выражение\n");
        fflush(stdout);
    } else {
        printf("ERROR: некорректное выражение (в стеке осталось %d элементов)\n", sp);
        fflush(stdout);
    }
}

int main(void)
{
    char line[LINE_SIZE + 2];

    while (fgets(line, sizeof(line), stdin) != NULL) {
        size_t len = strlen(line);
        int too_long = 0;

        if (len > 0 && line[len - 1] == '\n') {
            if (len - 1 > LINE_SIZE) {
                too_long = 1;
            }
        } else {
            if (len > LINE_SIZE) {
                too_long = 1;
            }
        }

        if (too_long) {
            printf("ERROR: слишком длинная строка\n");
            fflush(stdout);

            if (len > 0 && line[len - 1] != '\n') {
                while (fgets(line, sizeof(line), stdin) != NULL) {
                    if (strchr(line, '\n') != NULL) {
                        break;
                    }
                }
            }
            continue;
        }

        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            if (len > 1 && line[len - 2] == '\r') {
                line[len - 2] = '\0';
            }
        }

        if (strcmp(line, "quit") == 0) {
            break;
        }

        process_line(line);
    }

    return 0;
}

