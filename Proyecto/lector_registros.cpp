#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "registro.h"	// LIBRERÍA QUE DEFINE EL FORMATO DE UN REGISTRO

using namespace std;

int main(int argc, char const *argv[])
{	
	// ABRIENDO ARCHIVO PARA LECTURA DE REGISTROS
	int registros_leer_fd;
	if((registros_leer_fd = open("servidor_reg.dat", O_RDONLY)) == -1)
	{
		perror("registros.dat");
		exit(-1);
	}

	int bytes_leidos = 0;
	struct registro_time prueba;
	int i = 0;
	while(i < 2)
	{
		if((bytes_leidos = read(registros_leer_fd, &prueba, sizeof(struct registro_time))) > 0)
		{
			printf("Bytes leidos: %d\n", bytes_leidos);
			printf("REGISTRO\n");
			printf("Cel: %s\n", prueba.celular);
			printf("CURP: %s\n", prueba.CURP);
			printf("Partido: %s\n", prueba.partido);
			printf("Segundos: %ld\n", prueba.timestamp.tv_sec);
			printf("Micro segundos: %ld\n\n", prueba.timestamp.tv_usec);
		}				

		i++;
	}

	close(registros_leer_fd);

	return 0;
}