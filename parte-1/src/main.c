#include <stdio.h>
#include "../../parse/header.h"
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(void) {
    char *line;
    pid_t pid;
    int status;
    char prompt[1024];
    char cwd[512];
    char hostname[256];

    while (1) {
        // Promp Bonito
        char *user = getenv("USER");
        if (user == NULL) user = "user";

        if (getcwd(cwd, sizeof(cwd)) == NULL) strcpy(cwd, "?");
        if (gethostname(hostname, sizeof(hostname)) != 0) strcpy(hostname, "?");

        snprintf(prompt, sizeof(prompt), "%s@%s:%s$ ", user, hostname, cwd);

        line = readline(prompt);

        if (line == NULL) {
            printf("\nExiting...\n");
            exit(0);
        }
        if (strcmp(line, "exit") == 0) {
            free(line);
            exit(0);
        }
        if (*line) {
            add_history(line);
        }

        parse_line(line); //Eval

        int background = 0; // Flag

        if (command_list != NULL && command_list->name != NULL) {
            // Encontrar o '&'
            int i = 0;
            while (command_list->argv[i] != NULL) {
                i++;
            }
            if (i > 0 && strcmp(command_list->argv[i - 1], "&") == 0) {
                background = 1;
                command_list->argv[i - 1] = NULL; // Remover por causo do execvp
            }

            //Comandos Built-in
            if (strcmp(command_list->name, "cd") == 0) {
                char *destino = command_list->argv[1];
                if (destino == NULL) destino = getenv("HOME");
                if (chdir(destino) != 0) perror("cd");
            } else if (strcmp(command_list->name, "pwd") == 0) {
                char cwd_buf[1024];
                if (getcwd(cwd_buf, sizeof(cwd_buf)) != NULL) printf("%s\n", cwd_buf);
                else perror("pwd failed");
            } else if (strcmp(command_list->name, "history") == 0) {
                HIST_ENTRY **the_list = history_list();
                if (the_list) {
                    for (int j = 0; the_list[j]; j++) {
                        printf("%d: %s\n", j + 1, the_list[j]->line);
                    }
                }
            } else if (strcmp(command_list->name, "echo") == 0) {
                for (int j = 1; command_list->argv[j] != NULL; j++) {
                    printf("%s ", command_list->argv[j]);
                }
                printf("\n");
            } else if (strcmp(command_list->name, "help") == 0) {
                printf("Minishell Built-ins:\n");
                printf("  cd <dir>  - Altera a diretoria de trabalho\n");
                printf("  pwd       - Mostra o caminho absoluto atual\n");
                printf("  history   - Lista dos últimos comandos\n");
                printf("  echo      - Apresenta texto no terminal\n");
                printf("  exit      - Termina a minishell\n");
                printf("  help      - Instruções de utilização\n");
            }
            // Commandos externos
            else {
                if ((pid = fork()) == -1) {
                    perror("Error ao fazer fork");
                } else if (pid == 0) {
                    execvp(command_list->name, command_list->argv);
                    perror("execvp");
                    _exit(127);
                } else {
                    if (background) {
                        printf("[Processo em background iniciado com PID %d]\n", pid);
                    } else {
                        waitpid(pid, &status, 0);
                    }
                }
            }
        }

        free_command_list();
        free(line);
    }
    return 0;
}
