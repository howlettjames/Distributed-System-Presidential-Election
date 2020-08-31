#include <iostream>
#include <iterator>
#include "PaqueteDatagrama.h"

using namespace std;

PaqueteDatagrama::PaqueteDatagrama(char *mensaje, unsigned int longitudd, char *direccion, int puertoo)
{
	longitud = longitudd;
	datos = new char[longitud];
	memcpy(datos, mensaje, longitud);
	memcpy(ip, direccion, strlen(direccion));
	ip[strlen(direccion)] = '\0';
	puerto = puertoo;
}

PaqueteDatagrama::PaqueteDatagrama(unsigned int longitudd)
{
	longitud = longitudd;
	datos = new char[longitud];
}

PaqueteDatagrama::~PaqueteDatagrama()
{
	delete []datos;
}

char * PaqueteDatagrama::obtieneDireccion()
{
	return ip;
}

unsigned int PaqueteDatagrama::obtieneLongitud()
{
	return longitud;
}

int PaqueteDatagrama::obtienePuerto()
{
	return puerto;
}

char * PaqueteDatagrama::obtieneDatos()
{
	return datos;
}

void PaqueteDatagrama::inicializaPuerto(int ppuerto)
{
	puerto = ppuerto;
}

void PaqueteDatagrama::inicializaDireccion(char *direccion)
{
	memcpy(ip, direccion, strlen(direccion));
}

void PaqueteDatagrama::inicializaDatos(char *datoss)
{
	memcpy(datos, datoss, sizeof(datoss)/sizeof(datoss[0]));
}


