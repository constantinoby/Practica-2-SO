#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define _POSIX_C_SOURCE 200112L

#define DEBUG 1
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

/*_________________________________________________________________________________________________

                        DECLARACIONES DE FUNCIONES Y ESTRUCTURAS DE DATOS                                          
_________________________________________________________________________________________________*/

char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs(char **args);
int internal_fg(char **args);
int internal_bg(char **args);

void imprimir_prompt();


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

    printf(ROJO_T "%s:" RESET GRIS_T "%s" RESET AMARILLO_T " %c: " RESET user, prompt, PROMPT);
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


int parse_args(char **args, char *line)
{
    int tokens=0;
    const char s[5] = " \t\r\n";
    char *token;

    token = strtok(line, s);
    args[tokens]=token;

    while(token != NULL)
    {

#if DEBUG
        printf("[parse_args() → token %d: %s]\n", tokens, token);
#endif

        //Quitamos los comentarios
        if(*(token) != '#')
        {
            args[tokens]= token;
        }
        else
        {
            //Añadimos el NULL
            args[tokens]=NULL;
#if DEBUG
            printf("[parse_args() → token %d arreglado: %s]\n", tokens, token);
#endif
        }

        //Leemos el siguiente token
        token = strtok(NULL, s);
        tokens++;

    }
    return tokens;

}

int check_internal(char **args)
{
    int comando_interno = 0;

    if(strcmp(args[0], "cd") == 0)
    {
        internal_cd(args);
        comando_interno = 1;
    }
    else if(strcmp(args[0], "export") == 0)
    {
        internal_export(args);
        comando_interno = 1;
    }
    else if(strcmp(args[0], "source") == 0)
    {
        internal_source(args);
        comando_interno = 1;
    }
    else if(strcmp(args[0], "jobs") == 0)
    {
        internal_jobs(args);
        comando_interno = 1;
    }
    else if(strcmp(args[0], "bg") ==0)
    {
        internal_bg(args);
        comando_interno = 1;
    }
    else if(strcmp(args[0], "fg") ==0)
    {
        internal_fg(args);
        comando_interno = 1;
    }
    else if(strcmp(args[0],"exit") == 0)
    {
       exit(0);
    }

    return comando_interno;
}


int internal_cd(char **args)
{
#if DEBUG
    printf("[internal_cd() → Esta función cambiará de directorio]\n");
#endif
    return 1;
}

int internal_export(char **args)
{
#if DEBUG
    printf("[internal_export() → Esta función asignará valores a variables de entorno]\n");
#endif
    return 1;
}

int internal_source(char **args)
{
#if DEBUG
    printf("[internal_source() → Esta función ejecutará un fichero de líneas de comandos]\n");
#endif
    return 1;
}

int internal_jobs(char **args)
{
#if DEBUG
    printf("[internal_jobs() → Esta función mostrará el PID de los procesos que no estén en foreground]\n");
#endif
    return 1;
}

int internal_fg(char **args)
{
#if DEBUG
    printf("[internal_fg() → Esta función parsará/activará a primer plano procesos]\n");
#endif
    return 1;
}

int internal_bg(char **args)
{
#if DEBUG
    printf("[internal_bg() → Esta función parsará/activará a segundo plano procesos]\n");
#endif
    return 1;
}

