#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define VALOR 30

typedef struct datos
{
    char campo[10];
    char valor[50];
    char item_id[VALOR][15];
    char articulo[VALOR][60];
    char producto[VALOR][60];
    char marca[VALOR][60];
    //char tamanio[5];
} consulta;

int main(int argc, char* argv[])
{
    char *ayuda="-Help"; //Uso esta cadena para ver si el usuario quiere ver la ayuda
    if (argc == 2 && strcmp(argv[1],ayuda) == 0) //Muestro la ayuda al usuario
    {
        printf("\nEste programa tiene la funcionalidad de actuar como un Servidor que recibirá consultas");
        printf("\nLo que el usuario debe hacer es enviar un archivo de articulos, el cual sera abierto por el programa para realizar consultas");
        printf("\nLuego, este programa quedará esperando por una consulta. Al recibirla, leerá el archivo y buscará todas las coindicendias que existan");
        printf("\nLo que debe hacer es invocar al proceso con './Ejercicio4' (sin las comillas) y luego pasarle el archivo donde se encuentran los articulos");
        printf("\nEjemplo de ejecucion: ./Ejercicio4 /home/usuario/Desktop/articulos.txt");
        printf("\n");
        exit(3);
    }

    if (argc == 1)   //verifico que me haya pasado un archivo como parametro
    {
        printf("\nPor favor, pase el archivo de articulos");
        printf("\nEscriba './Ejercicio4 -Help' (sin las comillas) para recibir ayuda");
        printf("\n");
        exit(1);
    }
    if (argc >= 3)   //verifico que no me haya pasado mas parametros de los que pedi
    {
        printf("\nExceso de parámetros");
        printf("\nEscriba './Ejercicio4 -Help' (sin las comillas) para recibir ayuda");
        printf("\n");
        exit(1);
    }
    
    key_t Clave;                        //clave para recursos compartidos
    int Id_Memoria;                     //Identificador de Memoria
    int Id_Semaforo;                    //Identificador de Semaforos
    //consulta* Memoria = NULL;           //Memoria compartida por los procesos
    
    struct sembuf Operacion;                //Defino la forma de tratar a los semáforos en este proceso
    struct sembuf OperacionOtroProceso;     //Defino la forma de tratar a los semáforos en el otro proceso

    char* nombreArchivo= argv[1];   //Guardo el nombre del archivo

    FILE *archivo;                      //creo el puntero al archivo
    archivo=fopen(nombreArchivo,"r");   //lo abro en modo lectura
    if (archivo == NULL)                //verifico que el archivo exista y tenga permisos de lectura
    {
        printf("\nEl archivo no existe o no tiene permisos de lectura\n");
        exit(2);
    }

    char delimitador[]=";\n";             //con este delimitador y la funcion strtok voy a separar los campos
    char palabra[200];                  //guardaré una línea de archivos

    Clave = ftok("/bin/ls",VALOR);       //Pido una clave para recursos compartidos y verifico que haya podido recibirla
    if (Clave == (key_t) -1)
	{
		printf("No consigo clave para memoria compartida o semáforos\n");
		exit(0);
    }

    Id_Memoria = shmget (Clave, sizeof(consulta *)*1000, IPC_CREAT | 0666); //Pido ID para memoria compartida
    if (Id_Memoria == -1)
	{
		printf("No consigo Id para memoria compartida\n");
		exit (0);
    }
    
    Id_Semaforo = semget (Clave, 2, 0600 | IPC_CREAT);      //Pido ID para Semaforos y verifico que haya podido recibirla
	if (Id_Semaforo ==(key_t) -1)
	{
		printf("No puedo crear sem�foro\n");
		exit (0);
    }
    
    //Creo mi espacio de memoria compartida
    consulta* Memoria = (consulta *)shmat (Id_Memoria, (consulta *)0, 0);
    if(Memoria == NULL){
        printf("No pude conseguir memoria compartida\n");
        exit(0);
    }

    //Con esto voy a decrementar mi semáforo
    Operacion.sem_num= 0;
    Operacion.sem_op= -1;
    Operacion.sem_flg= 0;

    //Con esto voy a incrementar el semáforo del otro proceso
    OperacionOtroProceso.sem_num= 1;
    OperacionOtroProceso.sem_op= 1;
    OperacionOtroProceso.sem_flg= 0;

    char* item_id;
    char* articulo;
    char* producto;
    char* marca;

    while (1 == 1)
    {
        printf("\nEsperando consultas\n");
        semop(Id_Semaforo,&Operacion,1);                                     //Hago un P() a mi semàforo. No va a entrar hasta recibir consultas
        printf("\nConsulta recibida: %s-%s\n",Memoria->campo,Memoria->valor);
        //int tam = atoi(Memoria->tamanio);
        int tam = 0;
        while (feof(archivo) == 0)
        {
            fgets(palabra,200,archivo);
            item_id = strtok(palabra,delimitador);
            articulo = strtok(NULL,delimitador);
            producto = strtok(NULL,delimitador);
            marca = strtok(NULL,delimitador);
            if (strcmp(Memoria->campo,"ID") == 0)
            {
                if (strcmp(Memoria->valor,item_id) == 0)
                {
                    strcpy(Memoria->item_id[tam],item_id);
                    strcpy(Memoria->articulo[tam],articulo);
                    strcpy(Memoria->producto[tam],producto);
                    strcpy(Memoria->marca[tam],marca);
                    tam++;
                }
            }
            else
            {
                if (strcmp(Memoria->campo,"ARTICULO") == 0)
                {
                    if (strcmp(Memoria->valor,articulo) == 0)
                    {
                        strcpy(Memoria->item_id[tam],item_id);
                        strcpy(Memoria->articulo[tam],articulo);
                        strcpy(Memoria->producto[tam],producto);
                        strcpy(Memoria->marca[tam],marca);
                        tam++;
                    }
                }
                else
                {
                    if (strcmp(Memoria->campo,"PRODUCTO") == 0)
                    {
                        if (strcmp(Memoria->valor,producto) == 0)
                        {
                            strcpy(Memoria->item_id[tam],item_id);
                            strcpy(Memoria->articulo[tam],articulo);
                            strcpy(Memoria->producto[tam],producto);
                            strcpy(Memoria->marca[tam],marca);
                            tam++;
                        }
                    }
                    else
                    {
                        if (strcmp(Memoria->valor,marca) == 0)
                        {
                            /*puts("\nEncontre una coincidencia :D");
                            puts("\nDATOS DEL ARCHIVO:");
                            printf("\nID: %s", item_id);
                            printf("\nArticulo: %s", articulo);
                            printf("\nProducto: %s", producto);
                            printf("\nMarca: %s", marca);
                            puts("\n\nVamos a escribir en memoria compartida");*/
                            strcpy(Memoria->item_id[tam],item_id);
                            //printf("\nEscribi correctamente el id: %s\n", Memoria->item_id[tam]);
                            strcpy(Memoria->articulo[tam],articulo);
                            //printf("\nEscribi correctamente el articulo: %s\n", Memoria->articulo[tam]);
                            strcpy(Memoria->producto[tam],producto);
                            //printf("\nEscribi correctamente el producto: %s\n", Memoria->producto[tam]);
                            strcpy(Memoria->marca[tam],marca);
                            //printf("\nEscribi correctamente la marca: %s\n", Memoria->marca[tam]);
                            tam++;
                        }
                    }
                }
            }
        }
        strcpy(Memoria->item_id[tam],"cero");
        rewind(archivo);                             //le hago un rewind al archivo para poder leer otra vez
        //sprintf(Memoria->tamanio,"%d",tam);
        semop(Id_Semaforo,&OperacionOtroProceso,1); //hago un V() al semàforo del otro proceso
    }
   
    fclose(archivo);                    //cierro el archivo

    shmdt ((char *)Memoria);                                    //sirve para eliminar los recursos de memoria compartida
    shmctl (Id_Memoria, IPC_RMID, (struct shmid_ds *)NULL);     //sirve para eliminar los recursos de memoria compartida

    return 0;
}