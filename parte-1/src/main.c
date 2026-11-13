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
        line = readline("msh$ "); //LER

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


    return 0;
}
