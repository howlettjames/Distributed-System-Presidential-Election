#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "SocketDatagrama.h"

using namespace std;

SocketDatagrama::SocketDatagrama(int puerto)
{
	s = socket(AF_INET, SOCK_DGRAM, 0);

	int reuse = 1;
	if(setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) == -1)
	{
		printf("Error al utilizar la opcion reuseport\n");
		exit(EXIT_FAILURE);
	}

	bzero((char *)&direccionLocal, sizeof(direccionLocal));
	direccionLocal.sin_family = AF_INET;
	direccionLocal.sin_addr.s_addr = INADDR_ANY;
	direccionLocal.sin_port = htons(puerto);

	if(bind(s, (struct sockaddr *) &direccionLocal, sizeof(direccionLocal)) != 0)
	{
		cout << "Error: cannot bind" << endl;
		exit(0);
	}
	// bind(s, (struct sockaddr *) &direccionLocal, sizeof(direccionLocal));
}

void SocketDatagrama::setBroadcast()
{
	int yes = 1;
	if(setsockopt(s, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(int)) < 0)
	{
		printf("Error: No se pudo poner la opcion de Broadcast\n");
		exit(-1);
	}
}

int SocketDatagrama::recibe(PaqueteDatagrama &p)
{
	unsigned int clilen = sizeof(direccionForanea);

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout));

	int nBytesRecv = recvfrom(s, p.obtieneDatos(), p.obtieneLongitud(), 0, (struct sockaddr *)&direccionForanea, &clilen);
	p.inicializaPuerto(ntohs(direccionForanea.sin_port));
	bzero(p.obtieneDireccion(), 16);
	p.inicializaDireccion(inet_ntoa(direccionForanea.sin_addr));

	return nBytesRecv;
}

int SocketDatagrama::recibeTimeout(PaqueteDatagrama &p, time_t segundos, suseconds_t microsegundos)
{
	unsigned int clilen = sizeof(direccionForanea);

	timeout.tv_sec = segundos;
	timeout.tv_usec = microsegundos;

	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout));

	int nBytesRecv = recvfrom(s, p.obtieneDatos(), p.obtieneLongitud(), 0, (struct sockaddr *)&direccionForanea, &clilen);

	if(nBytesRecv < 0)
	{
		// if(errno == EWOULDBLOCK)
			// fprintf(stderr, "Tiempo de recepción transcurrido\n");
		// else
			// fprintf(stderr, "Error en recvfrom\n");
	}
	else
	{
		p.inicializaPuerto(ntohs(direccionForanea.sin_port));
		bzero(p.obtieneDireccion(), 16);
		p.inicializaDireccion(inet_ntoa(direccionForanea.sin_addr));	
	}
	
	return nBytesRecv;
}

int SocketDatagrama::envia(PaqueteDatagrama &p)
{
	bzero((char *)&direccionForanea, sizeof(direccionForanea));
	direccionForanea.sin_family = AF_INET;
	direccionForanea.sin_addr.s_addr = inet_addr(p.obtieneDireccion());
	direccionForanea.sin_port = htons(p.obtienePuerto());

	return sendto(s, (char *) p.obtieneDatos(), p.obtieneLongitud(), 0, (struct sockaddr *) &direccionForanea, sizeof(direccionForanea));
}

SocketDatagrama::~SocketDatagrama()
{
	close(s);
}
