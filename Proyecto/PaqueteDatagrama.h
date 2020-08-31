#ifndef PAQUETE_DATAGRAMA_H_
#define PAQUETE_DATAGRAMA_H_

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h> //memcpy

using namespace std;

class PaqueteDatagrama
{
	public:
		PaqueteDatagrama(char *, unsigned int, char *, int);
		PaqueteDatagrama(unsigned int);
		~PaqueteDatagrama();
		char *obtieneDireccion();
		unsigned int obtieneLongitud();
		int obtienePuerto();
		char *obtieneDatos();
		void inicializaPuerto(int);
		void inicializaDireccion(char *);
		void inicializaDatos(char *);

	private:
		char *datos;
		char ip[16];
		unsigned int longitud;
		int puerto;
};

#endif