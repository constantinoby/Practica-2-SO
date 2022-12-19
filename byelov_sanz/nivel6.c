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
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

#define DEBUG1 0          // Debug del parse_args
#define DEBUG2 0          // Debug del internal_export
#define DEBUG3 0          // Debug del internal_cd
#define DEBUG4 0          // Debug del execute_line
#define DEBUG5 0          // Debug del ctrlc() y reaper()
#define DEBUG6 1          // Debug del fg y bg
#define PROMPT_PERSONAL 1 // Si vale 1, se muestra el prompt personalizado

#define COMMAND_LINE_SIZE 1024
#define PROMPT '$'
#define ARGS_SIZE 64
#define N_JOBS 64
int n_pids = 0; // Contador de pids

#define RESET "\033[0m"
#define NEGRO_T "\x1b[30m"
#define NEGRO_F "\x1b[40m"
#define GRIS_T "\x1b[90m"
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
void reaper(int signum);
void ctrlc(int signum);
void ctrlz(int signum);

int is_background(char **args);
int jobs_list_find(pid_t pid);
int jobs_list_add(pid_t pid, char status, char *cmd);
int jobs_list_remove(int pos);

int is_background(char **args);
char *add_background(char *line);
int remove_background(char *line);
int is_output_redirection(char **args);

static char mi_shell[COMMAND_LINE_SIZE];

struct info_job
{
    pid_t pid;
    char status;                 // ‘N’, ’E’, ‘D’, ‘F’ (‘N’: ninguno, ‘E’: Ejecutándose y ‘D’: Detenido, ‘F’: Finalizado)
    char cmd[COMMAND_LINE_SIZE]; // línea de comando asociada
};

static struct info_job jobs_list[N_JOBS];

/*
---------------------------------------------------------------------------------------------------
PROGAMA PRINCIPAL
---------------------------------------------------------------------------------------------------
*/
int main(int argc, char *argv[])
{
    char line[COMMAND_LINE_SIZE];

    jobs_list[0].pid = 0; // Inicializo jobs
    jobs_list[0].status = 'N';
    memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);

    strcpy(mi_shell, argv[0]); // Guardamos el comando utilizado en mi_shell
    n_pids++;

    // añadimos para que escuche las señales
    signal(SIGCHLD, reaper);
    signal(SIGINT, ctrlc);
    signal(SIGTSTP, ctrlz);

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

#if PROMPT_PERSONAL == 1
    // Prompt con nombre de minishell
    printf(ROJO_T "%s" BLANCO_T ":", getenv("USER"));
    printf(AZUL_T "MINISHELL" BLANCO_T "%c " RESET, PROMPT);
#else
    // Nuestro prompt personalizado
    printf(ROJO_T "%s:" GRIS_T "%s" AMARILLO_T " %c: " RESET, getenv("USER"), getenv("PWD"), PROMPT);
#endif

    fflush(stdout);
    return;
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

    char *ptr = fgets(line, COMMAND_LINE_SIZE, stdin);

    if (!ptr)
    {
        printf("\r");
        if (feof(stdin)) // se ha pulsado Ctrl+D
        {
            fprintf(stderr, "Bye bye\n");
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
    pid_t pid;
    char copyLine[COMMAND_LINE_SIZE];

    memset(copyLine, '\0', sizeof(copyLine));
    strcpy(copyLine, line);

    if (parse_args(args, line) > 0)
    {
        int background = is_background(args);
        if (check_internal(args))
        {
            return 1;
        }
        else
        {
            // en caso de no ser commando interno, creamos hijo con fork()
            pid = fork();
            if (pid == 0) // CASO HIJO
            {

                // asociamos la accion por defecto a SIGCHLD
                signal(SIGCHLD, SIG_DFL);
                // indicar que se ignore la señal SIGINT
                signal(SIGINT, SIG_IGN);
                // indicar que se ignore la señal SIGTSTP
                signal(SIGTSTP, SIG_IGN);
                // comprobamos la redireccion
                is_output_redirection(args);
                // llamamos al execvp()
                execvp(args[0], args);
                // si el comando falla
                fprintf(stderr, ROJO_T "%s: no se ha encontró la orden\n", line);
                exit(-1);
            }
            else if (pid > 0) // CASO PADRE
            {

                // si el comando se va a ejecutar en foreground entonces...
                if (background == 0)
                {

                    // Actualizamos datos
                    jobs_list[0].pid = pid;
                    jobs_list[0].status = 'E';
                    strcpy(jobs_list[0].cmd, copyLine);

#if DEBUG4
                    fprintf(stderr, GRIS_T "[execute_line()→ PID padre: %d (%s)]\n" RESET, getpid(), mi_shell);
                    fprintf(stderr, GRIS_T "[execute_line()→ PID hijo: %d (%s)]\n" RESET, pid, copyLine);
#endif

                    // mientras haya un proceso hijo ejecutandose en foreground esperará (bloqueado)
                    while (jobs_list[0].pid > 0)
                    {
                        pause();
                    }
                }
                else
                {
#if DEBUG4
                    fprintf(stderr, GRIS_T "[execute_line()→ PID padre: %d (%s)]\n" RESET, getpid(), mi_shell);
                    fprintf(stderr, GRIS_T "[execute_line()→ PID hijo: %d (%s)]\n" RESET, pid, copyLine);
#endif
                    jobs_list_add(pid, 'E', copyLine);
                }
            }
            else // en caso contrario lanzamos error en fork()
            {
                perror("Error en fork");
                exit(-1);
            }
        }

        if (jobs_list_find(pid) != -1)
        {
            fprintf(stderr, RESET "[%d] %d\t%c\t%s\n", jobs_list_find(pid), jobs_list[jobs_list_find(pid)].pid, jobs_list[jobs_list_find(pid)].status, jobs_list[jobs_list_find(pid)].cmd);
        }
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
int parse_args(char **args, char *line)
{
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

    if (strcmp(args[0], "cd") == 0)
    {
        internal_cd(args);
        comando_interno = 1;
    }
    else if (strcmp(args[0], "export") == 0)
    {
        internal_export(args);
        comando_interno = 1;
    }
    else if (strcmp(args[0], "source") == 0)
    {
        internal_source(args);
        comando_interno = 1;
    }
    else if (strcmp(args[0], "jobs") == 0)
    {
        internal_jobs(args);
        comando_interno = 1;
    }
    else if (strcmp(args[0], "bg") == 0)
    {
        internal_bg(args);
        comando_interno = 1;
    }
    else if (strcmp(args[0], "fg") == 0)
    {
        internal_fg(args);
        comando_interno = 1;
    }
    else if (strcmp(args[0], "exit") == 0)
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
    path = malloc(COMMAND_LINE_SIZE); // Asignamos un espacio de memoria al path que nos van a pasar por comando
    if (!path)
    {
        fprintf(stderr, ROJO_T "No hay espacio de memoria dinámica" RESET);
        return -1;
    }

    if (args[1] != NULL)
    { // Tenemos un segundo argumento?

        for (int i = 1; args[i] != NULL; i++)
        { // Creamos una linea con todos los argumentos
            strcat(path, args[i]);
            if (args[i + 1] != NULL)
            {
                strcat(path, " ");
            }
        }

        eraseC(path); // Eliminamos los valores especiales

        if (chdir(path) < 0)
        { // Si el cambio de directorio va mal
            perror("chdir() Error: ");

            strcpy(path, "\0"); // Limpiamos memoria

            return -1;
        }
        strcpy(path, "\0"); // Limpiamos memoria
        free(path);
    }
    else if (chdir(getenv("HOME")) < 0)
    { // Si no nos pasan parametros cambiamos de directorio a HOME
        perror("chdir() Error: ");
        return -1;
    }

    // Cambiamos el PWD tras recibir el pathing correcto
    char cwd[COMMAND_LINE_SIZE];

    if (getcwd(cwd, sizeof(cwd)) == NULL)
    { // Asignamos el nuevo path al PWD
        perror("getcwd() Error: ");
        return -1;
    }
    else
    {
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
FUNCION: reaper()

ACCION: manejador propio para la señal SIGCHLD

SALIDA: no devuelve nada
---------------------------------------------------------------------------------------------------
*/
void reaper(int signum)
{
    // colocamos la instruccion signal() asociandola a esta misma funcion
    signal(SIGCHLD, reaper);

    int ended, pos;
    pid_t status;

    while ((ended = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (ended != jobs_list[0].pid)
        {
            // busqueda del pid del proceso en jobs_list
            pos = jobs_list_find(ended);

#if DEBUG5
            char mensaje[1200];
            if (status == 9)
            {
                sprintf(mensaje, GRIS_T "[reaper() -> Proceso hijo %d (%s) en background finalizado con señal %d]\n", ended, jobs_list[pos].cmd, status);
            }
            else
            {
                sprintf(mensaje, GRIS_T "\n[reaper() -> Proceso hijo %d (%s) en background finalizado con exit code %d]\n", ended, jobs_list[pos].cmd, status);
            }
            write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
#endif

            // mensaje de proceso finalizado
            char mensaje1[1200];
            sprintf(mensaje1, RESET "\nTerminado PID %d (%s) en jobs_list[%i] con status %i\n", jobs_list[pos].pid, jobs_list[pos].cmd, pos, status);
            write(2, mensaje1, strlen(mensaje1));

            // Eliminar el proceso de jobs_list
            jobs_list_remove(pos);
        }
        // si el hijo que ha finalizado se ejecutaba en foreground...
        else
        {

#if DEBUG5
            char mensaje[1200];
            sprintf(mensaje, GRIS_T "[reaper()→ Proceso hijo %d (%s) en foreground finalizado con exit code %d]\n" RESET, ended, jobs_list[0].cmd, status);
            write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
#endif
            // reseteamos datos jobs_list[0]
            jobs_list[0].pid = 0;
            jobs_list[0].status = 'N';
            strcpy(jobs_list[0].cmd, "");
        }
    }
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: ctrlc()

ACCION: manejador propio para la señal SIGINT

SALIDA: no devuelve nada
---------------------------------------------------------------------------------------------------
*/
void ctrlc(int signum)
{
    // ponemos la funcion signal asociandola a esta misma funcion
    signal(SIGINT, ctrlc);
    fprintf(stderr, GRIS_T "\n");
#if DEBUG5
    // imprimimos un salto de linea
    fflush(stdout);
    fprintf(stderr, GRIS_T "[ctrlc()→ Soy el proceso con PID %d (%s), el proceso en foreground es %d (%s)]\n" RESET, getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
#endif

    // si hay un proceso en foreground entonces...
    if (jobs_list[0].pid > 0)
    {
        // si el proceso en foreground no es el mini shell entonces...
        if (strcmp(jobs_list[0].cmd, mi_shell) != 0)
        {
            // enviamos señal SIGTERM
            kill(jobs_list[0].pid, SIGTERM);
#if DEBUG5
            fprintf(stderr, GRIS_T "[ctrlc()→ Señal %i (SIGTERM) enviada a %d (%s) por %d (%s)]\n" RESET, SIGTERM, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
#endif
        }
        // si no, lanzamos error
        else
        {
#if DEBUG5
            fprintf(stderr, GRIS_T "[ctrlc()→ Señal %i (SIGTERM) no enviada por %d (%s) debido a que su proceso en foreground es el shell]\n" RESET, SIGTERM, getpid(), mi_shell);
#endif
        }
    }
    // si no hay un proceso en foreground lanzamos error
    else
    {
#if DEBUG5
        fprintf(stderr, GRIS_T "[ctlrc()→Señal %i (SIGTERM) no enviada por %d (%s) debido a que no hay proceso en foreground]\n" RESET, SIGTERM, getpid(), mi_shell);
#endif
    }
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
    if (args[1]) // Si tenemos un parametro lo separamos
    {
        args[1] = strtok(args[1], "="); // Variable de entorno
        args[2] = strtok(NULL, "=");    // Valor de la variable de entorno
    }

    if (args[1] == NULL || args[2] == NULL) // Si alguna de las variables es NULL tiramos error de sintaxis
    {
        fprintf(stderr, ROJO_T " Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
    }
    else // Si no es NULL exportamos la variable
    {
#if DEBUG2
        fprintf(stderr, GRIS_T "[internal_export()→ nombre : %s]\n" RESET, args[1]);
        fprintf(stderr, GRIS_T "[internal_export()→ valor : %s]\n" RESET, args[2]);
        fprintf(stderr, GRIS_T "[internal_export()→ antiguo valor para USER : %s]\n" RESET, getenv(args[1]));
#endif

        setenv(args[1], args[2], 1); // sobreescribe el entorno actual con los parametros que se le pasan

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
    if (args[1] != NULL)
    { // Comprobamos sintaxis

        FILE *file;
        file = fopen(args[1], "r"); // Abrimos el archivo en modo lectura
        if (file != NULL)
        {
            char buffer[COMMAND_LINE_SIZE];
            while (fgets(buffer, COMMAND_LINE_SIZE, file) != NULL) // Leemos linea a linea el archivo
            {
                if (fflush(file) != 0)
                {
                    fprintf(stderr, "Error de flushing [%s]", args[1]);
                    return -1;
                }
                execute_line(buffer); // Ejecutamos la linea
            }
            if (fclose(file) != 0)
            {
                fprintf(stderr, "Error de cierre [%s]", args[1]);
                return -1;
            }
        }
        else
        {
            fprintf(stderr, ROJO_T "fopen: No such file or directory\n" RESET);
            return -1;
        }
    }
    else
    { // Si no se cumple la sintaxis tiramos error
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: source <nombre_fichero>\n" RESET);
        return -1;
    }

    return 1;
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: internal_jobs()

ACCION: Verifica si se trata del comando interno "jobs". En tal caso, mostrará el pid de los
        procesos que no esten en foreground.

PARAMETROS:
    - **args: Recibe el comando que se recoje por teclado

SALIDA: devuelve 1 si se corresponde con el comando interno "jobs"
---------------------------------------------------------------------------------------------------
*/
int internal_jobs(char **args)
{
    for (int i = 1; i < n_pids; i++)
    {
        printf("[%d] %d\t%c\t%s\n", i, jobs_list[i].pid, jobs_list[i].status, jobs_list[i].cmd);
    }
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

    int pos;

    pos = atoi(args[1]);
    if (pos < n_pids && pos >= 1)
    {
        // si status es D enviamos la señal SIGCONT
        if (jobs_list[pos].status == 'D')
        {
            kill(jobs_list[pos].pid, SIGCONT);
#if DEBUG6
            fprintf(stderr, GRIS_T "[internal_fg()→ Señal %i enviada a %d (%s)]\n", SIGCONT, jobs_list[pos].pid, jobs_list[pos].cmd);
#endif
        }
        // Copiar los datos de jobs_list[pos] a jobs_list[0], habiendo eliminado previamente del
        // cmd el ' &' (en caso de que lo tuviera), y cambiando el estado a 'E'.
        jobs_list[0].pid = jobs_list[pos].pid;
        jobs_list[0].status = 'E';
        remove_background(jobs_list[pos].cmd);
        strcpy(jobs_list[0].cmd, jobs_list[pos].cmd);
        // Eliminar jobs_list[pos] utilizando la función jobs_list_remove().
        jobs_list_remove(pos);
        // mostrar por pantalla el cmd eliminado '&'
        printf("%s\n", jobs_list[0].cmd);
        // pausamos mientras haya un proceso en ejecucion en foreground
        while (jobs_list[0].pid > 0)
        {
            pause();
        }
    }
    else
    {
        fprintf(stderr, "fg(%s): no existe este trabajo.\n", args[1]);
    }
    return 1;
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: internal_bg()

ACCION: Verifica si se trata del comando interno "bg". En tal caso reactivara un proceso detenido
        para que siga ejecutandose pero en background

PARAMETROS:
    - **args: Recibe el comando que se recoje por teclado

SALIDA: devuelve 1 si se corresponde con el comando interno "bg"
---------------------------------------------------------------------------------------------------
*/
int internal_bg(char **args)
{

    int pos;

    pos = atoi(args[1]);
    if (pos < n_pids && pos >= 1)
    {
        // Si el status de jobs_list[pos] es 'E' entonces error("el trabajo ya está en 2º plano") y salir.
        if (jobs_list[pos].status == 'E')
        {
            fprintf(stderr, "bg(%s)→ El trabajo ya está en segundo plano\n", args[1]);
        }
        else
        {
            // Cambiar el status de ese trabajo a 'E'
            jobs_list[pos].status = 'E';
            // agregar ' &' a su cmd
            strcpy(jobs_list[pos].cmd, add_background(jobs_list[pos].cmd));
            // Enviar a jobs_list[pos].pid la señal SIGCONT y provisionalmente notificarlo por pantalla.
            kill(jobs_list[pos].pid, SIGCONT);
#if DEBUG6
            fprintf(stderr, GRIS_T "[internal_bg()→ Señal %i enviada a %d (%s)]\n", SIGCONT, jobs_list[pos].pid, jobs_list[pos].cmd);
#endif
            // mostrar por pantalla el numero de trabajo, el PID, el estado y el cmd
            printf("[%d] %d\t%c\t%s\n", pos, jobs_list[pos].pid, jobs_list[pos].status, jobs_list[pos].cmd);
        }
    }
    else
    {
        fprintf(stderr, "bg(%s): no existe este trabajo\n", args[1]);
    }

    return 1;
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: is_background()

ACCION: busca si existe el token & (background), es decir, comprueba si el comando se deberia de
        ejecutar en background

PARAMETROS:
    - **args: Recibe el comando que se recoje por teclado

SALIDA: Devuelve 1 si ecuentra & o 0 en caso contrario
---------------------------------------------------------------------------------------------------
*/
int is_background(char **args)
{
    int encontrado = 0;
    int i;

    // nos situamos sobre el final del array de Strings
    for (i = 0; args[i]; i++)
    {
    }

    // comprobamos si es de background
    if (i > 1 && args[i - 1][0] == '&')
    {
        args[i - 1] = NULL; // eliminamos el token & del array de Strings
        encontrado = 1;
    }
    else if (i > 2 && args[i - 1][0] == '#' && args[i - 2][0] == '&')
    {
        args[i - 2] = NULL;
        encontrado = 1;
    }

    return encontrado;
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: add_background()

ACCION: agrega'&' al cmd

PARAMETROS:
    - *cmd: recibe el cmd del cual se ha de agregar '&'

SALIDA: Devuelve 1 si agrega & o 0 en caso contrario
---------------------------------------------------------------------------------------------------
*/
char *add_background(char *cmd)
{
    int n;
    char *provisional;

    // reservamos memoria a provisional
    provisional = malloc(COMMAND_LINE_SIZE);
    // si se reverva bien memoria continuamos...
    if (provisional)
    {
        // nos situamos sobre el ultimo indice
        n = strlen(cmd) - 1;
        while (cmd[n] == ' ')
        {
            // decrementamos una unidad
            n--;
        }
        // copiamos cmd correspondiente a provisional
        strcpy(provisional, cmd);
        if (cmd[n] != '&')
        {
            strcat(provisional, " &");
        }
    }
    // si no, lanzamos error
    else
    {
        perror("Error en add_background (reserva memoria)");
    }
    return provisional;
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: is_output_redirection()

ACCION: Recorre la lista de argumentos buscando '>' seguido del nombre del fichero. Si lo encuenta
        substituye el argumento '>' por NULL.

PARAMETROS:
    - **args: Recibe el comando que se recoje por teclado

SALIDA: Devuelve 1 si lo encuentra o 0 en caso contrario
---------------------------------------------------------------------------------------------------
*/
int is_output_redirection(char **args)
{
    int encontrado = 0;
    int fd;

    for (int i = 0; args[i]; i++) // args[i]!=NULL
    {
        // Buscamos el token '>'
        if (strlen(args[i]) == 1 && args[i][0] == '>' && args[i + 1])
        {
            // establecemos enlace logico con el fichero
            fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            // si el enlace logico con el fihcero se ha establecido de forma correcta, podemos continuar
            if (fd != -1)
            {
                dup2(fd, 1);
                args[i] = NULL;
                encontrado = 1;
                i--;
                // cierre del enlace logico con el fichero
                close(fd);
            }
            // si no, lanzamos error
            else
            {
                perror("No se ha podido establecer enlace con el fichero");
            }
        }
    }
    return encontrado;
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: jobs_list_add()

ACCION: añade un trabajo a la lista de trabajos

PARAMETROS:
    - pid: Recibe el pid del trabajo
    - status: Recibe el estado del trabajo
    - *cmd: Recibe el comando del trabajo

SALIDA: Devuelve 0 si se ha añadido correctamente y devuelve 1 no hay más sitio para nuevos
        trabajos
---------------------------------------------------------------------------------------------------
*/
int jobs_list_add(pid_t pid, char status, char *cmd)
{
    if (n_pids < N_JOBS)
    {
        jobs_list[n_pids].pid = pid;
        jobs_list[n_pids].status = status;
        strcpy(jobs_list[n_pids].cmd, cmd);
        n_pids++;
        return 0;
    }
    else
    {
        fprintf(stderr, "Maximum background jobs reached\n");
        return 1;
    }
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: jobs_list_find()

ACCION: busca un trabajo en la lista de trabajos

PARAMETROS:
    - pid: Recibe el pid del trabajo

SALIDA: Devuelve la posición del trabajo en la lista de trabajos y devuelve -1 si ha ocurrido un
        error
---------------------------------------------------------------------------------------------------
*/
int jobs_list_find(pid_t pid)
{
    int posicion = -1;

    for (int i = 1; i < n_pids; i++)
    {
        if (jobs_list[i].pid == pid)
        {
            posicion = i;
            // para salir del bucle
            i = n_pids;
        }
    }
    // si falla devuelve -1 y si no, devuelve la posicion
    return posicion;
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: jobs_list_remove()

ACCION: elimina un trabajo de la lista de trabajos

PARAMETROS:
    - pos: Recibe la posición donde se encuentra el trabajo en la lista de trabajos

SALIDA: Devuelve 0 si se ha eliminado el trabajo correctamente y devuelve 1 si ha ocurrido un error
---------------------------------------------------------------------------------------------------
*/
int jobs_list_remove(int pos)
{
    if (pos < n_pids && pos >= 1)
    {
        jobs_list[pos].pid = jobs_list[n_pids - 1].pid;
        jobs_list[pos].status = jobs_list[n_pids - 1].status;
        strcpy(jobs_list[pos].cmd, jobs_list[n_pids - 1].cmd);
        n_pids--;
        return 0;
    }
    return 1;
}
/*
---------------------------------------------------------------------------------------------------
FUNCION: ctrlz()

ACCION: manejador propio para la señal SIGTSTP

SALIDA: no devuelve nada
---------------------------------------------------------------------------------------------------
*/
void ctrlz(int signum)
{
    // volvemos a asociar este manejador a la señal SIGTSTP
    signal(SIGTSTP, ctrlz);
    fprintf(stderr, GRIS_T "\n");
#if DEBUG5
    // imprimimos un salto de linea

    fflush(stdout);
    fprintf(stderr, GRIS_T "[ctrlz()→ Soy el proceso con PID %d , el proceso en foreground es %d (%s)]\n" RESET, getpid(), jobs_list[0].pid, jobs_list[0].cmd);
#endif

    // si hay un proceso en foreground...
    if (jobs_list[0].pid > 0)
    {
        // si el proceso en foreground no es el mini shell...
        if (strcmp(jobs_list[0].cmd, mi_shell))
        {
            // enviar la señal SIGSTOP
            kill(jobs_list[0].pid, SIGSTOP);
#if DEBUG5
            fprintf(stderr, GRIS_T "[ctrlz()→ Señal %i (SIGSTOP) enviada a %d (%s) por %d (%s)]\n" RESET, SIGSTOP, jobs_list[n_pids - 1].pid, jobs_list[n_pids - 1].cmd, getpid(), mi_shell);
#endif
            // cambiar los datos del proceso detenido usando jobs_list_add()
            jobs_list_add(jobs_list[0].pid, 'D', jobs_list[0].cmd);
            // resetear los datos de jobs_list
            jobs_list[0].pid = 0;
            jobs_list[0].status = 'N';
            strcpy(jobs_list[0].cmd, "");
        }
        // si no, mensaje de error
        else
        {
#if DEBUG5
            fprintf(stderr, GRIS_T "[ctrlz()→ Señal %i no enviada por %d (%s) debido a que su proceso en foreground es el shell]\n" RESET, SIGSTOP, getpid(), mi_shell);
#endif
        }
    }
    // si no, mensaje de error
    else
    {
#if DEBUG5
        fprintf(stderr, GRIS_T "[ctlrz()→Señal %i no enviada por %d (%s) debido a que no hay proceso en foreground]\n" RESET, SIGSTOP, getpid(), mi_shell);
#endif
    }
}

/*
---------------------------------------------------------------------------------------------------
METODOS DE USUARIO
---------------------------------------------------------------------------------------------------
*/

int eraseC(char *line)
{

    char s[4] = "\\\"\'";
    char res[COMMAND_LINE_SIZE];
    int res_i = 0;
    int found;
    int cambio = 0;

    for (int i = 0; line[i] != '\0'; i++)
    { // Recorremos la linea de comandos que nos ha llegado.
        found = 0;
        for (int j = 0; j < 3 && (found == 0); j++)
        { // Recorremos todos os separadores.
            if (line[i] == s[j])
            {
                found = 1;  // Encontramos el caracter separador.
                cambio = 1; // Indicamos que se ha habido cambio en la linea.
            }
        }
        if (found == 0)
        { // Si no hemos encontrado un valor de los posibles separadores, copiamos la linea.
            res[res_i] = line[i];
            res_i++;
        }
    }
    res[res_i] = '\0'; // Se termina la linea de resultado.

    strcpy(line, res); // Pasamos la linea modificada.
    return cambio;
}

/*
---------------------------------------------------------------------------------------------------
FUNCION: remove_background()

ACCION: Elimina '&' del cmd si tiene '&'

PARAMETROS:
    - *cmd: recibe el cmd del cual se ha de eliminar '&'

SALIDA: Devuelve 1 si elimina & o 0 en caso contrario
---------------------------------------------------------------------------------------------------
*/
int remove_background(char *cmd)
{
    for (int i = 0; cmd[i] != '\0'; i++)
    {
        if (cmd[i] == '&')
        {
            cmd[i] = '\0';
            return 1;
        }
    }
    return 0;
}