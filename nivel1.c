/*_________________________________________________________________________________________________
PRACTICA 2

SQUAD: Ctrl Unit

AUTORES: 
         Nicolás Tuñón Sanz
         Constantino Byelov Serdiuk
___________________________________________________________________________________________________

#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

char line[COMMAND_LINE_SIZE];

/*
---------------------------------------------------------------------------------------------------
PROGAMA PRINCIPAL
---------------------------------------------------------------------------------------------------
*/
int main()
{

        while (1)
    {
        if (read_line(line))
        {
            execute_line(line);
        }
    }

    return 0;
    
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: imprimir_prompt()

ACCION: muestra el prompt por pantalla

PARAMETROS: no tiene

SALIDA: no devuelve nada
---------------------------------------------------------------------------------------------------
*/
void imprimir_prompt()
{
    char *user = getenv("USER");

    char *home= getenv("HOME");

    printf(ROJO_T "%s:"  GRIS_T "%s"  AMARILLO_T " %c: " RESET ,user, home, PROMPT);
    

    fflush(stdout);
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: read_line() 

ACCION: Se encarga de leer la linea que se introduce por teclado

PARAMETROS: 
    - *line: Recibe el array donde se almacenara la linea leida

SALIDA: devuelve el puntero a la linea leida
---------------------------------------------------------------------------------------------------
*/
char *read_line(char *line)
{

    imprimir_prompt();

    if(fgets(line, COMMAND_LINE_SIZE, stdin) == NULL)
    {
        perror("Error al leer la linea");
        
    }

    return line;
}


/*
---------------------------------------------------------------------------------------------------
FUNCION: execute_line()

ACCION: Lleva a cabo la ejecucion de la linea de comandos introducida por el usuario. 

PARAMETROS: 
    - *line: Recibe la linea leida por teclado

SALIDA: devuelve 1 si se corresponde con un comando interno o 0 si es externo
---------------------------------------------------------------------------------------------------
*/
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
---------------------------------------------------------------------------------------------------
FUNCION: parse_args() 

ACCION: se encarga de trocear la linea en tokens. Va visualizando de forma momentanea los tokens

PARAMETROS: 
    - **args: Recibe el comando que se recoje por teclado
    - *line: Recibe la linea leida por teclado

SALIDA: devuelve el numero de tokens que tiene la linea
---------------------------------------------------------------------------------------------------
*/
int parse_args(char **args, char *line) {
    int i = 0;

    const char s[5] = " \t\r\n";

    args[i] = strtok(line, s);

    #if DEBUG 
        fprintf(stderr, GRIS_T "[parse_args()→ token %i: %s]\n" RESET, i, args[i]);
    #endif

    while (args[i] && args[i][0] != '#') // args[i]!= NULL && *args[i]!='#'
    { 
        i++;
        args[i] = strtok(NULL, s);

        #if DEBUG 
            fprintf(stderr, GRIS_T "[parse_args()→ token %i: %s]\n" RESET, i, args[i]);
        #endif

    }
    if (args[i]) 
    {
        args[i] = NULL; // por si el último token es el símbolo comentario

        #if DEBUG 
            fprintf(stderr, GRIS_T "[parse_args()→ token %i corregido: %s]\n" RESET, i, args[i]);
        #endif
        
    }
    return i;
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: check_internal()

ACCCION: Verifica si el comando que recibe por parametro es interno o externo 

PARAMETROS:
    - **args: Recibe el comando que se recoje por teclado para identificar si es interno o externo

SALIDA: Devuelve 1 si es un comando interno o 0 si es externo.
---------------------------------------------------------------------------------------------------
*/
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

/*
---------------------------------------------------------------------------------------------------
FUNCION: internal_cd() (NO IMPLEMENTADA)

ACCION: Verifica si se trata del comando interno "cd"

PARAMETROS: 
    - **args: Recibe el comando que se recoje por teclado

SALIDA: devuelve 1 si se corresponde con el comando interno "cd"
---------------------------------------------------------------------------------------------------
*/
int internal_cd(char **args)
{
#if DEBUG
    printf("[internal_cd() → Esta función cambiará de directorio]\n");
#endif
    return 1;
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: internal_export() (NO IMPLEMENTADA)

ACCION: Verifica si se trata del comando interno "export"

PARAMETROS: 
    - **args: Recibe el comando que se recoje por teclado

SALIDA: devuelve 1 si se corresponde con el comando interno "export"
---------------------------------------------------------------------------------------------------
*/
int internal_export(char **args)
{
#if DEBUG
    printf("[internal_export() → Esta función asignará valores a variables de entorno]\n");
#endif
    return 1;
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: internal_source() (NO IMPLEMENTADA)

ACCION: Verifica si se trata del comando interno "source"

PARAMETROS: 
    - **args: Recibe el comando que se recoje por teclado

SALIDA: devuelve 1 si se corresponde con el comando interno "source"
---------------------------------------------------------------------------------------------------
*/
int internal_source(char **args)
{
#if DEBUG
    printf("[internal_source() → Esta función ejecutará un fichero de líneas de comandos]\n");
#endif
    return 1;
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: internal_jobs() (NO IMPLEMENTADA)

ACCION: Verifica si se trata del comando interno "jobs". 

PARAMETROS: 
    - **args: Recibe el comando que se recoje por teclado

SALIDA: devuelve 1 si se corresponde con el comando interno "jobs"
---------------------------------------------------------------------------------------------------
*/
int internal_jobs(char **args)
{
#if DEBUG
    printf("[internal_jobs() → Esta función mostrará el PID de los procesos que no estén en foreground]\n");
#endif
    return 1;
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: internal_fg() (NO IMPLEMENTADA)

ACCION: Verifica si se trata del comando interno "cd".

PARAMETROS: 
    - **args: Recibe el comando que se recoje por teclado

SALIDA: devuelve 1 si se corresponde con el comando interno "fg"
---------------------------------------------------------------------------------------------------
*/
int internal_fg(char **args)
{
#if DEBUG
    printf("[internal_fg() → Esta función parsará/activará a primer plano procesos]\n");
#endif
    return 1;
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: internal_bg() (NO IMPLEMENTADA)

ACCION: Verifica si se trata del comando interno "bg".

PARAMETROS: 
    - **args: Recibe el comando que se recoje por teclado

SALIDA: devuelve 1 si se corresponde con el comando interno "bg"
---------------------------------------------------------------------------------------------------
*/
int internal_bg(char **args)
{
#if DEBUG
    printf("[internal_bg() → Esta función parsará/activará a segundo plano procesos]\n");
#endif
    return 1;
}
