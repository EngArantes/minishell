/*
 *  main.c
 *  parse
 */

#include "header.h"
#include <unistd.h>

int main(int argc, const char *argv[]) {
    char *line;
    char prompt[1024];  // Buffer do prompt
    char cwd[512];      // Buffer do cwd
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
            printf("\nExiting...\n");
            break;
        }
        
        // Verifica se a linha digitada é "exit"
        if (strcmp(line, "exit") == 0) {
            free(line); // Libertação da memória de readline antes de sair
            break;
        }
        
        // Adiciona à história caso a linha não esteja vazia
        if (*line) {
            add_history(line);
        }
        
        parse_line(line);
        execute_commands();
        free_command_list();
        
        free(line); // Liberta a memória alocada por readline
    }
    
    return 0;
}
