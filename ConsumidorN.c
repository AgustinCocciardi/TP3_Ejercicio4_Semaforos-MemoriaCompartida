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
        printf("\nEste programa tiene la funcionalidad de permitirle al usuario enviar consultas al proceso Servidor");
        printf("\nLo que el usuario debe hacer es enviar una consulta al proceso servidor y luego mostrar el resultado por pantalla");
        printf("\nLo que debe hacer es invocar al proceso con './Consumidor' (sin las comillas) y luego pasarle la consulta a realizar");
        printf("\nEjemplo de ejecucion: ./Consumidor PRODUCTO=HELADO");
        printf("\n");
        exit(3);
    }

    if (argc == 1)   //verifico que me haya pasado una consulta
    {
        printf("\nPor favor, pase una consulta");
        printf("\nEscriba './Consumidor -Help' (sin las comillas) para recibir ayuda");
        printf("\n");
        exit(1);
    }
    if (argc >= 3)   //verifico que no me haya pasado mas parametros de los que pedi
    {
        printf("\nExceso de parámetros");
        printf("\nEscriba './Consumidor -Help' (sin las comillas) para recibir ayuda");
        printf("\n");
        exit(1);
    }

    char* consultaInput= argv[1]; //Guardo en una variable lo que recibi como parametro

    char delimitador[] = "="; 

    char campo[10];
    char valor[50];

    strcpy(campo,strtok(consultaInput,delimitador));
    strcpy(valor,strtok(NULL,delimitador));

    if (strcmp(campo,"ID") != 0 && strcmp(campo,"MARCA") != 0 && strcmp(campo,"PRODUCTO") != 0 && strcmp(campo,"ARTICULO") != 0)
    {
        printf("\nSu consulta está mal hecha: NO HA PASADO CORRECTAMENTE EL CAMPO");
        printf("\n");
        exit(3);
    }

    key_t Clave;                    //clave para recursos compartidos
    int Id_Memoria;                 //Identificador de Memoria
    int Id_Semaforo;                //Identificador de Semaforos
	//consulta* Memoria = NULL;       //Memoria compartida por los procesos

    struct sembuf Operacion;                //Defino la forma de tratar a los semáforos en este proceso
    struct sembuf OperacionOtroProceso;     //Defino la forma de tratar a los semáforos en el otro proceso

    Clave = ftok("/bin/ls",VALOR);          //Pido una clave para recursos compartidos y verifico que haya podido recibirla
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

    semctl(Id_Semaforo,1,SETVAL,1);     //Inicializo el semáforo de mi proceso en 1

    //Con esto voy a decrementar mi semáforo
    Operacion.sem_num= 1;
    Operacion.sem_op= -1;
    Operacion.sem_flg= 0;

    //Con esto voy a incrementar el semáforo del otro proceso
    OperacionOtroProceso.sem_num= 0;
    OperacionOtroProceso.sem_op= 1;
    OperacionOtroProceso.sem_flg= 0;

    char num[2]="0";

    semop(Id_Semaforo,&Operacion,1);                        //Hago un P() de mi semáforo. Lo inicialicè en 1, asì que voy a entrar

    strcpy(Memoria->campo,campo);
    //puts(Memoria->campo);
    strcpy(Memoria->valor,valor);
    //puts(Memoria->valor);
    //strcpy(Memoria->tamanio,num);
    //puts(Memoria->tamanio);
    printf("\nSu consulta es\t Campo: %s Valor: %s\n", Memoria->campo, Memoria->valor);
    //Memoria->tamanio = 0;                                   //Inicializo las ocurrencias en 0
    //puts("Hasta aca llegue");
    semop(Id_Semaforo,&OperacionOtroProceso,1);             //Hago un V() al semáforo del otro proceso
    semop(Id_Semaforo,&Operacion,1);                        //Hago un P() de mi semaforo. Se supone que ya lo incrementó el otro proceso
    //int tam=atoi(Memoria->tamanio);
    int i=0;
    while (strcmp(Memoria->item_id[i],"cero") != 0)
    {
        printf("\nId: %s Articulo: %s Producto: %s Marca: %s", Memoria->item_id[i], Memoria->articulo[i], Memoria->producto[i], Memoria->marca[i]);
        printf("\n");
        i++;
    }
    
    shmdt ((char *)Memoria);            //sirve para eliminar los recursos de memoria compartida

    return 0;
}