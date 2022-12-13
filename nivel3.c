/*_________________________________________________________________________________________________
PRACTICA 2

SQUAD: Ctrl Unit

AUTORES: 
         Nicolás Sanz Tuñón
         Constantino Byelov Serdiuk
___________________________________________________________________________________________________
*/

#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define DEBUG1 0                // Debug del parse_args
#define DEBUG2 0                // Debug del internal_export
#define DEBUG3 1                // Debug del internal_cd
#define DEBUG4 1                // Debug del execute_line
#define PROMPT_PERSONAL 1       // Si vale 1, se muestra el prompt personalizado

#define COMMAND_LINE_SIZE 1024
#define PROMPT '$'
#define ARGS_SIZE 64
#define N_JOBS 64


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
int eraseC(char *line);

static char mi_shell[COMMAND_LINE_SIZE]; 

struct info_job {
   pid_t pid;
   char status; // ‘N’, ’E’, ‘D’, ‘F’ (‘N’: ninguno, ‘E’: Ejecutándose y ‘D’: Detenido, ‘F’: Finalizado) 
   char cmd[COMMAND_LINE_SIZE]; // línea de comando asociada
};

static struct info_job jobs_list [N_JOBS];

/*
---------------------------------------------------------------------------------------------------
PROGAMA PRINCIPAL
---------------------------------------------------------------------------------------------------
*/
int main(int argc, char *argv[])
{
    char line[COMMAND_LINE_SIZE];
    
    jobs_list[0].pid=0;                                         //Inicializo jobs
    jobs_list[0].status='N';
    memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);

    strcpy(mi_shell,argv[0]);                                   //Guardamos el comando utilizado en mi_shell

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
    
#if PROMPT_PERSONAL==1
    //Prompt con nombre de minishell
    printf(ROJO_T "%s" BLANCO_T ":", getenv("USER"));
    printf(AZUL_T "MINISHELL" BLANCO_T "%c " RESET, PROMPT);
#else
    //Nuestro prompt personalizado
    printf(ROJO_T "%s:"  GRIS_T "%s"  AMARILLO_T " %c: " RESET ,getenv("USER"), getenv("PWD"), PROMPT);
#endif

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

    char *ptr= fgets(line, COMMAND_LINE_SIZE, stdin);

    if(!ptr)
    {
        printf("\r");
        if (feof(stdin)) //se ha pulsado Ctrl+D
        { 
            fprintf(stderr,"Bye bye\n");
            exit(0);
        }
        
    }
    else
    {
        // ELiminamos el salto de línea (ASCII 10) sustituyéndolo por el \0
        char *pos = strchr(line, 10);
        if (pos != NULL)
        {
            *pos = '\0';
        } 
    }

    return ptr;
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
    pid_t pid, status;
    char copyLine[COMMAND_LINE_SIZE];

    memset(copyLine, '\0', sizeof(copyLine)); 
    strcpy(copyLine,line);


    if (parse_args(args, line)>0)
    {
       if(check_internal(args))
       {
        return 1;
       }
       else
       {
        //en caso de no ser commando interno, creamos hijo con fork()
        pid= fork();
        if(pid==0)//CASO HIJO
        {
            //llamamos al execvp()
            execvp(args[0],args);
            //si el comando falla 
            fprintf(stderr, ROJO_T "%s: no se ha encontró la orden\n",line);
            exit(-1);
        }
        else if(pid>0) //CASO PADRE
        {
            //Actualizamos datos
            jobs_list[0].pid=pid;
            jobs_list[0].status='E';
            strcpy(jobs_list[0].cmd,copyLine);
            
            #if DEBUG4
                fprintf(stderr, GRIS_T "[execute_line()→ PID padre: %d (%s)]\n" RESET, getpid(), mi_shell);
                fprintf(stderr, GRIS_T "[execute_line()→ PID hijo: %d (%s)]\n" RESET, pid, copyLine);
            #endif
            //esperamos a que terminen el hijo o los hijos
            pid=wait(&status);
        }
        else //en caso contrario lanzamos error en fork()
        {
            perror("Error en fork");
            exit(-1);
        }
       }
       //reseteamos jobs_list[0]
        jobs_list[0].pid=0;
        jobs_list[0].status='N';
        memset(jobs_list[0].cmd, '\0',COMMAND_LINE_SIZE);
        #if DEBUG4
            fprintf(stderr, GRIS_T "[execute_line()→ Proceso hijo %d (%s) se ha finalizado con exit(), estado: %d]\n" RESET,pid, copyLine, status);
        #endif

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

    #if DEBUG1 
        fprintf(stderr, GRIS_T "[parse_args()→ token %i: %s]\n" RESET, i, args[i]);
    #endif

    while (args[i] && args[i][0] != '#') // args[i]!= NULL && *args[i]!='#'
    { 
        i++;
        args[i] = strtok(NULL, s);

        #if DEBUG1 
            fprintf(stderr, GRIS_T "[parse_args()→ token %i: %s]\n" RESET, i, args[i]);
        #endif

    }
    if (args[i]) 
    {
        args[i] = NULL; // por si el último token es el símbolo comentario

        #if DEBUG1 
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
FUNCION: internal_cd() 

ACCION: Verifica si se trata del comando interno "cd", en tal caso cambia de repositorio dependiendo 
        de los parametros que le lleguen.

PARAMETROS: 
    - **args: Recibe el comando que se recoje por teclado

SALIDA: devuelve 1 si se corresponde con el comando interno "cd"
---------------------------------------------------------------------------------------------------
*/

int internal_cd(char **args)
{
    char *path;
    path = malloc(COMMAND_LINE_SIZE);                                 //Asignamos un espacio de memoria al path que nos van a pasar por comando
    if(!path){
        fprintf(stderr, ROJO_T "No hay espacio de memoria dinámica" RESET);
        return -1;
    }
    
    if(args[1] != NULL){                                                    // Tenemos un segundo argumento?

        for(int i = 1; args[i]!=NULL; i++){                                 // Creamos una linea con todos los argumentos
            strcat(path, args[i]);
            if(args[i+1]!=NULL){
                strcat(path, " ");
            }

        }

        eraseC(path);                                                       // Eliminamos los valores especiales
       
        if(chdir(path) < 0){                                                //Si el cambio de directorio va mal
            perror("chdir() Error: ");
             
            strcpy(path, "\0");                                             // Limpiamos memoria
            free(path);
            
            return -1;
        }
        strcpy(path, "\0");                                                 // Limpiamos memoria
        free(path);
    
    }else if(chdir(getenv("HOME")) < 0 ){                                   // Si no nos pasan parametros cambiamos de directorio a HOME
        perror("chdir() Error: ");
        return -1;
    } 

                                                                            // Cambiamos el PWD tras recibir el pathing correcto
    char cwd[COMMAND_LINE_SIZE];
    
    if(getcwd(cwd, sizeof(cwd)) == NULL){                                   //Asignamos el nuevo path al PWD
        perror("getcwd() Error: ");
        return -1;
    }else{
        setenv("PWD", cwd, 1);
    }

#if DEBUG3
    char *prompt;
    if ((prompt = malloc(COMMAND_LINE_SIZE)))
    {
        // Gets the current work directory.
        getcwd(prompt, COMMAND_LINE_SIZE);

        fprintf(stderr, GRIS_T "[internal_cd() → PWD: %s]\n" RESET, prompt);
    }
    else
    {
        perror("Error debug internal_cd()");
    }

    free(prompt);
#endif

    return 1;
}


/*
---------------------------------------------------------------------------------------------------
FUNCION: internal_export()

ACCION: Verifica si se trata del comando interno "export"

PARAMETROS: 
    - **args: Recibe el comando que se recoje por teclado

SALIDA: devuelve 1 si se corresponde con el comando interno "export"
---------------------------------------------------------------------------------------------------
*/
int internal_export(char **args)
{
  if (args[1])                                                              //Si tenemos un parametro lo separamos
  {
    args[1] = strtok(args[1], "=");                                         // Variable de entorno
    args[2] = strtok(NULL, "=");                                            // Valor de la variable de entorno
  }

    if (args[1]==NULL || args[2]==NULL)                                     //Si alguna de las variables es NULL tiramos error de sintaxis
    {
        fprintf(stderr, ROJO_T " Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
    }
    else                                                                   //Si no es NULL exportamos la variable
    {
        #if DEBUG2 
        fprintf(stderr, GRIS_T "[internal_export()→ nombre : %s]\n" RESET, args[1]);  
        fprintf(stderr, GRIS_T "[internal_export()→ valor : %s]\n" RESET, args[2]); 
        fprintf(stderr, GRIS_T "[internal_export()→ antiguo valor para USER : %s]\n" RESET, getenv(args[1]));      
        #endif

        setenv(args[1],args[2], 1);                                       // sobreescribe el entorno actual con los parametros que se le pasan

        #if DEBUG2 
        fprintf(stderr, GRIS_T "[internal_export()→ nuevo valor para USER : %s]\n" RESET, args[2]);        
        #endif

    }
    return 1;

}

/*
---------------------------------------------------------------------------------------------------
FUNCION: internal_source()

ACCION: Verifica si se trata del comando interno "source"

PARAMETROS: 
    - **args: Recibe el comando que se recoje por teclado

SALIDA: devuelve 1 si se corresponde con el comando interno "source"
---------------------------------------------------------------------------------------------------
*/
int internal_source(char **args)
{
    if(args[1] !=NULL){                                                    // Comprobamos sintaxis

        FILE *file;
        file=fopen(args[1], "r");                                          // Abrimos el archivo en modo lectura
        if(file !=NULL)
        {
            char buffer[COMMAND_LINE_SIZE];
            while(fgets(buffer, COMMAND_LINE_SIZE,file)!=NULL)             // Leemos linea a linea el archivo
            {
                if(fflush(file)!=0){
                    fprintf(stderr, "Error de flushing [%s]", args[1]);
                    return -1;
                }
                execute_line(buffer);                                   // Ejecutamos la linea

            }
            if(fclose(file)!=0){
                fprintf(stderr, "Error de cierre [%s]", args[1]);
                return -1;
            }
        }
        else
        {
            fprintf(stderr, ROJO_T "fopen: No such file or directory\n" RESET);
            return -1;
        }
    }else{                                                               // Si no se cumple la sintaxis tiramos error
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: source <nombre_fichero>\n" RESET);
        return -1;
    }
    
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

/*
---------------------------------------------------------------------------------------------------
METODOS DE USUARIO
---------------------------------------------------------------------------------------------------
*/

int eraseC(char *line){

    char s[4] = "\\\"\'";
    char res[COMMAND_LINE_SIZE];
    int res_i = 0;
    int found;
    int cambio = 0;
    
    for(int i = 0; line[i] != '\0'; i++){                                   // Recorremos la linea de comandos que nos ha llegado.
    found = 0;
        for(int j = 0; j < 3 &&(found == 0); j++){                          // Recorremos todos os separadores.
            if (line[i]==s[j]){
                found = 1;                                                  // Encontramos el caracter separador.
                cambio = 1;                                                 // Indicamos que se ha habido cambio en la linea.
            }
        } 
        if(found == 0){                                                     // Si no hemos encontrado un valor de los posibles separadores, copiamos la linea.
            res[res_i] = line[i]; 
            res_i++;
        }

    }
    res[res_i] = '\0';                                              //Se termina la linea de resultado.
    
    strcpy(line, res);                                              // Pasamos la linea modificada.
    return cambio;
}
