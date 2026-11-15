#include <stdio.h>
#include "../../parse/header.h"
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h> // Necessário para as flags open(), O_RDONLY, O_WRONLY, etc.

int main(void) {
    char *line;
    pid_t pid;
    int status;
    char prompt[1024];
    char cwd[512];
    char hostname[256];

    while (1) {
        // 1.1 Prompt Modificado
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

        parse_line(line); // Avalia a linha de comandos

        int background = 0; // Reset à flag de background para cada comando

        if (command_list != NULL && command_list->name != NULL) {
            // --- 1.3 BACKGROUND '&' ---
            int i = 0;
            while (command_list->argv[i] != NULL) {
                i++;
            }
            if (i > 0 && strcmp(command_list->argv[i - 1], "&") == 0) {
                background = 1;
                command_list->argv[i - 1] = NULL; // Remove '&' para o execvp o ignorar
            }

            // --- 1.2: COMANDOS BUILT-IN ---
            if (strcmp(command_list->name, "cd") == 0) {
                char *destino = command_list->argv[1];
                if (destino == NULL) destino = getenv("HOME");
                if (chdir(destino) != 0) perror("cd");
            } else if (strcmp(command_list->name, "pwd") == 0) {
                int saved_stdout = dup(STDOUT_FILENO);
                int fd_out = -1;
                if (command_list->outfile != NULL) {
                    int flags = O_WRONLY | O_CREAT | (command_list->append_out ? O_APPEND : O_TRUNC);
                    fd_out = open(command_list->outfile, flags, 0644);
                    if (fd_out >= 0) dup2(fd_out, STDOUT_FILENO);
                }

                char cwd_buf[1024];
                if (getcwd(cwd_buf, sizeof(cwd_buf)) != NULL) printf("%s\n", cwd_buf);
                else perror("pwd failed");

                fflush(stdout);
                if (fd_out >= 0) {
                    dup2(saved_stdout, STDOUT_FILENO);
                    close(fd_out);
                }
                close(saved_stdout);
            } else if (strcmp(command_list->name, "history") == 0) {
                int saved_stdout = dup(STDOUT_FILENO);
                int fd_out = -1;
                if (command_list->outfile != NULL) {
                    int flags = O_WRONLY | O_CREAT | (command_list->append_out ? O_APPEND : O_TRUNC);
                    fd_out = open(command_list->outfile, flags, 0644);
                    if (fd_out >= 0) dup2(fd_out, STDOUT_FILENO);
                }

                HIST_ENTRY **the_list = history_list();
                if (the_list) {
                    for (int j = 0; the_list[j]; j++) {
                        printf("%d: %s\n", j + 1, the_list[j]->line);
                    }
                }

                fflush(stdout);
                if (fd_out >= 0) {
                    dup2(saved_stdout, STDOUT_FILENO);
                    close(fd_out);
                }
                close(saved_stdout);
            } else if (strcmp(command_list->name, "echo") == 0) {
                int saved_stdout = dup(STDOUT_FILENO);
                int fd_out = -1;
                if (command_list->outfile != NULL) {
                    int flags = O_WRONLY | O_CREAT | (command_list->append_out ? O_APPEND : O_TRUNC);
                    fd_out = open(command_list->outfile, flags, 0644);
                    if (fd_out >= 0) dup2(fd_out, STDOUT_FILENO);
                }

                for (int j = 1; command_list->argv[j] != NULL; j++) {
                    printf("%s ", command_list->argv[j]);
                }
                printf("\n");

                fflush(stdout);
                if (fd_out >= 0) {
                    dup2(saved_stdout, STDOUT_FILENO);
                    close(fd_out);
                }
                close(saved_stdout);
            } else if (strcmp(command_list->name, "help") == 0) {
                int saved_stdout = dup(STDOUT_FILENO);
                int fd_out = -1;
                if (command_list->outfile != NULL) {
                    int flags = O_WRONLY | O_CREAT | (command_list->append_out ? O_APPEND : O_TRUNC);
                    fd_out = open(command_list->outfile, flags, 0644);
                    if (fd_out >= 0) dup2(fd_out, STDOUT_FILENO);
                }

                printf("Minishell Built-ins:\n");
                printf("  cd <dir>  - Altera a diretoria de trabalho\n");
                printf("  pwd       - Mostra o caminho absoluto atual\n");
                printf("  history   - Lista dos últimos comandos\n");
                printf("  echo      - Apresenta texto no terminal\n");
                printf("  exit      - Termina a minishell\n");
                printf("  help      - Instruções de utilização\n");

                fflush(stdout);
                if (fd_out >= 0) {
                    dup2(saved_stdout, STDOUT_FILENO);
                    close(fd_out);
                }
                close(saved_stdout);
            }
            // --- COMANDOS EXTERNOS ---
            else {
                if ((pid = fork()) == -1) {
                    perror("Error ao fazer fork");
                } else if (pid == 0) {
                    // --- 1.4: REDIRECIONAMENTOS (INPUT/OUTPUT) ---

                    // 1. Redirecionamento de Input (<)
                    if (command_list->infile != NULL) {
                        int fd_in = open(command_list->infile, O_RDONLY);
                        if (fd_in < 0) {
                            perror("Erro ao abrir ficheiro de input");
                            _exit(1);
                        }
                        dup2(fd_in, STDIN_FILENO); // Substitui o stdin pelo ficheiro
                        close(fd_in);
                    }

                    // 2. Redirecionamento de Output (> ou >>)
                    if (command_list->outfile != NULL) {
                        int flags = O_WRONLY | O_CREAT;
                        if (command_list->append_out == 1) {
                            flags |= O_APPEND; // >> (Anexar)
                        } else {
                            flags |= O_TRUNC; // >  (Sobrescrever)
                        }

                        // Permissões 0644: rw-r--r--
                        int fd_out = open(command_list->outfile, flags, 0644);
                        if (fd_out < 0) {
                            perror("Erro ao abrir ficheiro de output");
                            _exit(1);
                        }
                        dup2(fd_out, STDOUT_FILENO); // Substitui o stdout pelo ficheiro
                        close(fd_out);
                    }

                    // 3. Redirecionamento de Erro (2>)
                    if (command_list->errfile != NULL) {
                        // 2> sobrescreve sempre, logo usamos O_TRUNC
                        int fd_err = open(command_list->errfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (fd_err < 0) {
                            perror("Erro ao abrir ficheiro de erro");
                            _exit(1);
                        }
                        dup2(fd_err, STDERR_FILENO); // Substitui o stderr pelo ficheiro
                        close(fd_err);
                    }
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

        // Libertação de memória sempre executada para evitar leaks
        free_command_list();
        free(line);
    }

    return 0;
}
