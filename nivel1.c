
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define COMMAND_LINE_SIZE 1024
#define PROMPT '$'
#define ARGS_SIZE 64


#define RESET "\033[0m"
#define NEGRO_T "\x1b[30m"
#define NEGRO_F "\x1b[40m"
#define GRIS_T "\x1b[94m"
#define ROJO_T "\x1b[31m"
#define VERDE_T "\x1b[32m"
#define AMARILLO_T "\x1b[33m"
#define AZUL_T "\x1b[34m"
#define MAGENTA_T "\x1b[35m"
#define CYAN_T "\x1b[36m"
#define BLANCO_T "\x1b[97m"
#define NEGRITA "\x1b[1m"


int main()
{
    char line[COMMAND_LINE_SIZE];

        while (1)
    {
        if (read_line(line))
        {
            execute_line(line);
        }
    }

    return 0;
    
}


char *read_line(char *line)
{

    imprimir_prompt();

    if(fgets(line, COMMAND_LINE_SIZE, stdin) == NULL)
    {
        perror("Error al leer la linea");
    }

    return line;
}

void imprimir_prompt()
{
    char *user = getenv("USER");

    char *prompt = malloc(sizeof(char) * COMMAND_LINE_SIZE);

    getcwd(prompt, COMMAND_LINE_SIZE);

    printf(ROJO_T "%s:" RESET "%s" RESET AMARILLO_T" %c: "RESET user, prompt, PROMPT);

    free(prompt);

    fflush(stdout);
}

int execute_line(char *line)
{
    char *args[ARGS_SIZE];

    parse_args(args, line);
    if (args[0])
    {
       check_internal(args);
    }
    else
    {
        return -1;
    }

    return 0;
    
}

/*
int parse_args(char **args, char *line)
{

}

int check_internal(char **args)
{

}

*/