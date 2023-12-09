/** CONCEPTOS BASICOS **/
/**
 * Un mmap es un mapeo de memoria. Es una forma de compartir memoria entre procesos. 
 * Se utiliza para compartir archivos entre procesos, de tal forma que si un proceso modifica el archivo, los cambios se propagan a los demás procesos.
 * 
 * mmmap(void *dir, size_t len, int prot, int flags, int fd, off_t offset);
 * dir: dirección de memoria donde se mapea el archivo. Si es NULL, el sistema elige la dirección.
 * len: tamaño del mapeo en bytes.
 * prot: especifica los permisos de acceso al mapeo. Puede ser PROT_READ, PROT_WRITE, PROT_EXEC o una combinación de ellos.
 * flags: especifica si las modificaciones realizadas en el mapeo se propagan al archivo original. Puede ser MAP_PRIVATE o MAP_SHARED.
 * fd: descriptor del archivo que se mapea.
 * 
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>  // Para los flags de open
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>

#define BUFFER_SIZE 1024 // Tamaño del buffer

volatile sig_atomic_t continuarPadre = 0;
volatile sig_atomic_t continuarHijo = 0;

/**
 * Funcion que comprueba si un caracter es un numero
 * @param caracter Caracter a comprobar
 * @return 1 si es un numero, 0 si no lo es
 */
int esNumero(char caracter){
    if(caracter >= '0' && caracter <= '9'){
        return 1;
    }
    return 0;
}

/**
 * Funcion que gestiona la señal SIGUSR1
 * @param sig Señal recibida
*/
void signal_handlerHijo(int sig) {
    continuarHijo = 1;
}

/**
 * Funcion que gestiona la señal SIGUSR1
 * @param sig Señal recibida
*/
void signal_handlerPadre(int sig) {
    continuarPadre = 1;
}

int main(int argc, char *argv[]) {

    /////////////////////////** DECLARACION DE VARIABLES **/////////////////////////

    int fichero_entrada, fichero_salida; // Ficheros de entrada y salida
    pid_t hijo; // PID del proceso hijo
    off_t tamFichero = 0; // Tamaño del fichero de entrada

    
    /////////////////////////* GESTION DE ARGUMENTOS DE ENTRADA */////////////////////////

    if(argc != 3){
        printf("USO: ./ejecutable <nombre_fichero_entrada> <nombre_fichero_salida>\n");
        EXIT_FAILURE;
    }

    // Abrir fichero de entrada para lectura
    if ((fichero_entrada = open(argv[1], O_RDONLY)) == -1) { // La flag O_RDONLY abre el fichero para lectura
        perror("Error al abrir el fichero de entrada");
        exit(EXIT_FAILURE);
    }

    /////////////////////////** GESTION DE TAMAÑOS Y FICHEROS **/////////////////////////

    //Con lseek movemos el puntero del archivo
    tamFichero = lseek(fichero_entrada, 0, SEEK_END); // Obtener el tamaño del fichero de entrada
    lseek(fichero_entrada, 0, SEEK_SET); // Volver al principio del fichero

    //Las flags O_RDWR y O_CREAT abren el fichero para lectura y escritura y lo crea si no existe, mientras que O_TRUNC trunca el fichero a 0, y 
    //la flag 0600 establece los permisos de lectura y escritura para el usuario que ejecuta el programa
    fichero_salida = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0600); // Abrir fichero de salida para lectura y escritura
    if (fichero_salida < 0) { // Error al abrir el fichero de salida
        perror("Error al abrir el archivo de salida");
        close(fichero_entrada);
        return EXIT_FAILURE;
    }

    // Mapeos
    char *entrada = mmap(NULL, tamFichero, PROT_READ, MAP_PRIVATE, fichero_entrada, 0);

    // Calculamos el tamaño necesario para el archivo de salida
    off_t tamSalida = tamFichero; // Tamaño inicial
    for (off_t i = 0; i < tamFichero; ++i) {
        if (esNumero(entrada[i])) {
            int num = entrada[i] - '0';
            tamSalida += (num - 1); // Sumamos (num - 1) porque reemplazaremos un carácter (el número) por 'num' asteriscos.
        }
    }

    // Mapeo el archivo de salida con el nuevo tamaño
    char *salida = mmap(NULL, tamFichero, PROT_READ | PROT_WRITE, MAP_SHARED, fichero_salida, 0);

     if (entrada == MAP_FAILED || salida == MAP_FAILED) { // Error al mapear los archivos
        perror("Error al mapear los archivos");
        close(fichero_entrada);
        close(fichero_salida);
        return EXIT_FAILURE;
    }

    // Ajustar el tamaño del archivo de salida
    ftruncate(fichero_salida, tamSalida); 

    /////////////////////////** CUERPO DEL PROGRAMA **/////////////////////////

    // Crear proceso hijo
    hijo = fork();
    if(hijo < 0){ // Error al crear el proceso hijo
        perror("Error al crear el proceso hijo"); // Mostrar error
        close(fichero_entrada); // Cerrar fichero de entrada
        close(fichero_salida); // Cerrar fichero de salida
        return EXIT_FAILURE; // Salir del programa

    } else if(hijo == 0){ // Proceso hijo

        printf("Proceso hijo iniciado, pid del padre: %d\n\n", getppid());

        sleep(1);

        // Configurar el manejador de señales.
        signal(SIGUSR1, signal_handlerHijo);

        /* PRIMERA MITAD DEL ARCHIVO */

        // Esperar segunda señal para continuar
        while(!continuarHijo);
        continuarHijo = 0;

        sleep(1);

        printf("Procesando primera mitad del archivo (Hijo)\n\n");

        sleep(1);

         // Variables para mantener la posición actual en el archivo de salida
        off_t posSalida = 0;

        // Proceso hijo: Procesar la entrada y escribir en la salida (primera mitad)
        for (off_t i = 0; i < tamFichero / 2; i++) {
            if (esNumero(entrada[i])) {
                int num = entrada[i] - '0'; // Convertir el caracter a numero
                for (int j = 0; j < num; j++) {
                    salida[posSalida] = '*';
                    posSalida++;
                }
            }else{
                posSalida++;
            }
        }

        kill(getppid(), SIGUSR1); // Enviar señal al padre para que continue
        printf("Enviando señal al padre y esperando\n\n");
        sleep(2);

        /* SEGUNDA MITAD DEL ARCHIVO */

        // Esperar segunda señal del padre para procesar la segunda mitad
        while(!continuarHijo);
        continuarHijo = 0;

        sleep(1);

        printf("Procesando segunda mitad del archivo (Hijo)\n\n");

        sleep(1);

        posSalida = tamFichero / 2; // Volver a la mitad del archivo

        for (off_t i = tamFichero / 2; i < tamFichero; i++) { // Procesar la entrada y escribir en la salida (segunda mitad)
            if (esNumero(entrada[i])) { // Si es un numero
                int num = entrada[i] - '0'; // Convertir el caracter a numero
                for (int j = 0; j < num; j++) { // Escribir el numero de asteriscos
                    salida[posSalida] = '*';
                    posSalida++; // Incrementar la posición de salida
                }
            }else{
                posSalida++; // Incrementar la posición de salida
            }
        }

    } 
    else{ // Proceso padre
        // En el proceso padre, justo después de fork()
        printf("Proceso padre iniciado, pid del hijo: %d\n", hijo);

        sleep(1);

        // Configurar el manejador de señales.
        signal(SIGUSR1, signal_handlerPadre);

         /* PRIMERA MITAD DEL ARCHIVO */

        // Antes de procesar cada parte del archivo
        printf("Procesando primera mitad del archivo (Padre)\n\n");

        sleep(1);

        off_t posSalidaPadre = 0;

        // Procesar la entrada y escribir en la salida
        for (off_t i = 0; i < tamFichero/2; i++) { // Procesar la entrada y escribir en la salida (primera mitad)
            if (!esNumero(entrada[i])) { // Si no es un numero
                salida[posSalidaPadre] = toupper(entrada[i]); // Convertir a mayúscula
                posSalidaPadre++; // Incrementar la posición de salida
            } else if(esNumero(entrada[i])){ // Si es un numero
                int num = entrada[i] - '0'; // Convertir el caracter a numero
                for (int j = 0; j < num; j++) {
                    salida[posSalidaPadre] = ' '; // Escribir el numero de espacios
                    posSalidaPadre++; // Incrementar la posición de salida
                } 
            }else {
                salida[posSalidaPadre] = entrada[i]; // Si no es un numero ni una letra
                posSalidaPadre++; // Incrementar la posición de salida
            }
        }

        kill(hijo, SIGUSR1); // Enviar señal al hijo para que continue
        printf("Enviando señal al hijo y esperando\n\n");

        sleep(1);

        /* SEGUNDA MITAD DEL ARCHIVO */
        
        // Esperar segunda señal del padre para procesar la segunda mitad
        while(!continuarPadre);
        continuarPadre = 0;

        sleep(1);

        printf("Procesando segunda mitad del archivo (Padre)\n\n");

        sleep(1);

        posSalidaPadre = tamFichero/2;

        // Procesar la entrada y escribir en la salida
        for (off_t i = tamFichero/2; i < tamFichero; i++) {
            if (!esNumero(entrada[i])) { // Si no es un numero
                salida[posSalidaPadre] = toupper(entrada[i]); // Convertir a mayúscula
                posSalidaPadre++;
            } else if(esNumero(entrada[i])){
                int num = entrada[i] - '0'; // Convertir el caracter a numero
                for (int j = 0; j < num; j++) {
                    salida[posSalidaPadre] = ' ';
                    posSalidaPadre++;
                } 
            }else {
                salida[posSalidaPadre] = entrada[i];
                posSalidaPadre++;
            }
        }
    


        kill(hijo, SIGUSR1); // enviar señal al hijo
        printf("Enviando señal al hijo y esperando\n\n");

        wait(NULL); // Esperar a que el hijo termine

        printf("EXITO: archivo transformado\n");

        // Liberar memoria y cerrar ficheros
        msync(salida, tamSalida, MS_SYNC);
        close(fichero_entrada);
        close(fichero_salida);
    }

    return 0;
}
