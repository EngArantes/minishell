    #include <stdio.h>
    #include "../../parse/header.h"
    #include <sys/wait.h>
    #include <unistd.h>

    int main(void)
    {
        char *line;
        pid_t pid;
        int status;

        while (1)
        {
            line = readline("msh$ ");

            if (line == NULL)
            {
                printf("\nExiting...");
                exit(0);
            }
            if(strcmp(line,"exit") == 0)
            {
                free(line);
                exit(0);
            }
            if (*line)
            {
                add_history(line);
            }
            parse_line(line); //Eval


            if (command_list != NULL && command_list->name != NULL && strcmp(command_list->name, "cd") == 0)
            {
                char * destino = command_list->argv[1];
                if (destino == NULL)
                {
                    destino = getenv("HOME");
                }

                if (chdir(destino) != 0)
                {
                 perror("cd");
                }
                else
                {

                }

            }
            else
            {
                if ((pid = fork()) == -1)
                {
                    perror("Error ao fazer fork");
                }
                else if(pid == 0)
                {
                    execvp(command_list->name, command_list->argv);
                    perror("execvp");
                    _exit(127);
                }
                else
                {
                    waitpid(pid, &status, 0);
                }

                free_command_list();
                free(line);
            }
        }


        return 0;
    }
