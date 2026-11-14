#include "header.h"
#include <unistd.h>
#include <sys/wait.h>

void execute_commands(void) {
    // Vazio passa a frente
    if (command_list == NULL || command_list->name == NULL) {
        return;
    }

    CMD *cmd = command_list;

    // Comandos pedidos
    if (strcmp(cmd->name, "cd") == 0) {
        if (cmd->argv[1] != NULL) {
            if (chdir(cmd->argv[1]) != 0) perror("cd failed");
        } else {
            printf("cd: missing argument\n");
        }
        return;
    } 
    else if (strcmp(cmd->name, "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("pwd failed");
        }
        return;
    }
    else if (strcmp(cmd->name, "history") == 0) {
        HIST_ENTRY **the_list = history_list();
        if (the_list) {
            for (int i = 0; the_list[i]; i++) {
                printf("%d: %s\n", i + 1, the_list[i]->line);
            }
        }
        return;
    }
    else if (strcmp(cmd->name, "echo") == 0) {
        for (int i = 1; cmd->argv[i] != NULL; i++) {
            printf("%s ", cmd->argv[i]);
        }
        printf("\n");
        return;
    }
    else if (strcmp(cmd->name, "help") == 0) {
        printf("Minishell Built-ins:\n");
        printf("  cd <dir>  - Altera a diretoria de trabalho\n");
        printf("  pwd       - Mostra o caminho absoluto atual\n");
        printf("  history   - Lista dos últimos comandos\n");
        printf("  echo      - Apresenta texto no terminal\n");
        printf("  exit      - Termina a minishell\n");
        printf("  help      - Instruções de utilização\n");
        return;
    }
    else if (strcmp(cmd->name, "exit") == 0) {
        //So para garantir
        exit(0); 
    }

    // Comandos Externos
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
    } else if (pid == 0) {
        // Processo Filho: Substituido pelo novo programa
        if (execvp(cmd->name, cmd->argv) == -1) {
            perror(cmd->name);
            exit(EXIT_FAILURE);
        }
    } else {
        // Processo Pai: Aguarda o término do processo filho
        int status;
        waitpid(pid, &status, 0); 
    }
}