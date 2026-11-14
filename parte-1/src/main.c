#include <stdio.h>
#include "../../parse/header.h"
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    char *line;
    pid_t pid;
    int status;
    char prompt[1024]; // Buffer do prompt
    char cwd[512]; // Buffer do cwd
    char hostname[256]; // Buffer for user

    while (1) {
        // Promp bonito
        char *user = getenv("USER");
        if (user == NULL) user = "user";

        if (getcwd(cwd, sizeof(cwd)) == NULL) strcpy(cwd, "?");
        if (gethostname(hostname, sizeof(hostname)) != 0) strcpy(hostname, "?");

        snprintf(prompt, sizeof(prompt), "%s@%s:%s$ ", user, hostname, cwd);
        // --------------------------------------------------

        //Troca o msh$ pelo promp
        line = readline(prompt);

        // Trata o sinal EOF (Ctrl+D)
        if (line == NULL) {
            printf("\nExiting...");
            exit(0);
        }

        // Verifica se a linha digitada é "exit"
        if (strcmp(line, "exit") == 0) {
            free(line);
            exit(0);
        }

        // Adiciona à história caso a linha não esteja vazia
        if (*line) {
            add_history(line);
        }
        parse_line(line); //Eval


        if (command_list != NULL && command_list->name != NULL) {
            if (strcmp(command_list->name, "cd") == 0) {
                char *destino = command_list->argv[1];
                if (destino == NULL) {
                    destino = getenv("HOME");
                }

                if (chdir(destino) != 0) {
                    perror("cd");
                } else {
                }
            }
            else if (strcmp(command_list->name, "pwd") == 0) {
                char cwd[1024];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("%s\n", cwd);
                } else {
                    perror("pwd failed");
                }
            }
            else if (strcmp(command_list->name, "history") == 0) {
                HIST_ENTRY **the_list = history_list();
                if (the_list) {
                    for (int i = 0; the_list[i]; i++) {
                        printf("%d: %s\n", i + 1, the_list[i]->line);
                    }
                }
            }
            else if (strcmp(command_list->name, "echo") == 0) {
                for (int i = 1; command_list->argv[i] != NULL; i++) {
                    printf("%s ", command_list->argv[i]);
                }
                printf("\n");
            }
            else if (strcmp(command_list->name, "help") == 0) {
                printf("Minishell Built-ins:\n");
                printf("  cd <dir>  - Altera a diretoria de trabalho\n");
                printf("  pwd       - Mostra o caminho absoluto atual\n");
                printf("  history   - Lista dos últimos comandos\n");
                printf("  echo      - Apresenta texto no terminal\n");
                printf("  exit      - Termina a minishell\n");
                printf("  help      - Instruções de utilização\n");
            }
            else if (strcmp(command_list->name, "exit") == 0) {
                //So para garantir
                exit(0);
            }
        }else {
            if ((pid = fork()) == -1) {
                perror("Error ao fazer fork");
            } else if (pid == 0) {
                execvp(command_list->name, command_list->argv);
                perror("execvp");
                _exit(127);
            } else {
                waitpid(pid, &status, 0);
            }

            free_command_list();
            free(line);
        }
    }
}
