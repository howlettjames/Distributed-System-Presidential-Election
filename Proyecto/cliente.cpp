#include <iostream>
#include <thread>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "Solicitud.h"
#include "registro.h"	// LIBRERÍA QUE DEFINE EL FORMATO DE UN REGISTRO

using namespace std;

#define SERVIDOR_IP_1 "127.0.0.1"
// #define SERVIDOR_IP_1 "192.168.100.99" 		// Cel
// #define SERVIDOR_IP_3 "25.121.204.24"		// Jaime

// #define SERVIDOR_IP_2 "25.142.15.73"			// Alex
#define SERVIDOR_IP_2 "127.0.0.1"
// #define SERVIDOR_IP_2 "25.121.204.24"		// Jaime

// #define SERVIDOR_IP_3 "25.47.119.146" 		// Esteban
#define SERVIDOR_IP_3 "127.0.0.1"
// #define SERVIDOR_IP_3 "192.168.100.110" 		// Compu
// #define SERVIDOR_IP_3 "25.121.204.24"		// Jaime

#define SERVIDOR_PUERTO_1 7200
#define SERVIDOR_PUERTO_2 7201
#define SERVIDOR_PUERTO_3 7202

void funcion_hilo(int num_hilo, int registros_a_enviar)
{
	int registros_fd, bytes_leidos;
	struct mensaje *men_respuesta;
	struct registro reg;
	struct timeval timestamp;
	bool procesarRespuesta;
	
	// CREAMOS UN OBJETO SOLICITUD PARA HACER SOLICITUD DE OPERACIONES
	Solicitud solicitud;

	// ABRIENDO ARCHIVO PARA LECTURA DE REGISTROS
	string archivo_a_leer("registros.dat");
	if((registros_fd = open(archivo_a_leer.c_str(), O_RDONLY)) == -1)
	{
		perror("registros.dat");
		exit(-1);
	}

	// LEEMOS DEL ARCHIVO FUENTE DE REGISTROS
	while((bytes_leidos = read(registros_fd, &reg, sizeof(struct registro))) > 0 && registros_a_enviar > 0)
	{
		// printf("REGISTRO LEIDO\n");
		// printf("Cel: %s\n", reg.celular);
		// printf("CURP: %s\n", reg.CURP);
		// printf("Partido: %s\n", reg.partido);
		// printf("Datagrama enviado, esperando confirmacion del servidor...\n");

		procesarRespuesta = false;
		if(num_hilo == 1)
		{
			if(reg.celular[9] < '4')
			{
				// LLAMAMOS A LA OPERACIÓN REMOTA "GUARDAR REGISTRO", LA CUAL EN CASO DE SER EXITOSA NOS DEVOLVERÁ UN TIMESTAMP DE
				// CUANDO EL REGISTRO FUE ALMACENADO EN EL SERVIDOR, EN OTRO CASO INDICADO POR EL CAMPO "ID DE OPERACIÓN", SE HARÁ LO CORRESPONDIENTE
				men_respuesta = solicitud.doOperation(SERVIDOR_IP_1, SERVIDOR_PUERTO_1, guardar_registro, (char *) &reg, sizeof(struct registro));
				procesarRespuesta = true;
			}
		}
		else if(num_hilo == 2)
		{
			// if(reg.celular[9] > '4')
			if(reg.celular[9] > '3' && reg.celular[9] < '7')
			{
				men_respuesta = solicitud.doOperation(SERVIDOR_IP_2, SERVIDOR_PUERTO_2, guardar_registro, (char *) &reg, sizeof(struct registro));
				procesarRespuesta = true;
			}
		}
		else if(num_hilo == 3)
		{
			if(reg.celular[9] > '6' && reg.celular[9] <= '9')
			{
				men_respuesta = solicitud.doOperation(SERVIDOR_IP_3, SERVIDOR_PUERTO_3, guardar_registro, (char *) &reg, sizeof(struct registro));
				procesarRespuesta = true;
			}
		}

		if(procesarRespuesta)
		{
			if(men_respuesta->operationId == mensaje_repetido)
			{
				// SE TRATA DE UN MENSAJE REPETIDO QUE EL SERVIDOR YA HABÍA PROCESADO
				printf("%d: Mensaje del servidor: %s\n", num_hilo, men_respuesta->arguments);
			}
			else
			{
				// SABEMOS QUE RECIBIREMOS UNA ESTRUCTURA TIMEVAL
				memcpy(&timestamp, men_respuesta->arguments, sizeof(struct timeval));
				// IMPRIMIENDO MENSAJE DE CONFIRMACIÓN
				// printf("%d: Momento de transaccion de %s en el servidor: %ld : %ld\n", num_hilo, reg.celular, timestamp.tv_sec, timestamp.tv_usec);
			}
		}
		
		registros_a_enviar--;	
	}
	// CERRANDO EL ARCHIVO	
	close(registros_fd);

	// CERRANDO COMUNICACIÓN CON EL SERVIDOR CORRESPONDIENTE
	char mensaje_adios[] = "Cerrando comunicacion";
	if(num_hilo == 1)
		men_respuesta = solicitud.doOperation(SERVIDOR_IP_1, SERVIDOR_PUERTO_1, cerrando_comunicacion, mensaje_adios, strlen(mensaje_adios));
	else if(num_hilo == 2)
		men_respuesta = solicitud.doOperation(SERVIDOR_IP_2, SERVIDOR_PUERTO_2, cerrando_comunicacion, mensaje_adios, strlen(mensaje_adios));
	else if(num_hilo == 3)
		men_respuesta = solicitud.doOperation(SERVIDOR_IP_3, SERVIDOR_PUERTO_3, cerrando_comunicacion, mensaje_adios, strlen(mensaje_adios));

	// CHECANDO SI YA NO HAY MÁS REGISTROS O SI OCURRIÓ ALGÚN ERROR DE LECTURA
	if(registros_a_enviar == 0)
		printf("%d: Ya no hay mas registros por leer. Terminando el programa...\n", num_hilo);
	else if(bytes_leidos < 0)
		fprintf(stderr, "%d: Ocurrio un error de lectura. Terminando el programa...\n", num_hilo);
}

int main(int argc, char const *argv[])
{
	// CHECANDO QUE LA INVOCACIÓN SEA CORRECTA
	if(argc != 2)
	{
		printf("Forma de uso: %s <numero_registros_a_enviar>\n", argv[0]);
		exit(0);
	}

	int registros_a_enviar = atoi(argv[1]);
		
	thread hilo1(funcion_hilo, 1, registros_a_enviar);
	thread hilo2(funcion_hilo, 2, registros_a_enviar);
	thread hilo3(funcion_hilo, 3, registros_a_enviar);
	
	hilo1.join();
	hilo2.join();
	hilo3.join();

	return 0;
}