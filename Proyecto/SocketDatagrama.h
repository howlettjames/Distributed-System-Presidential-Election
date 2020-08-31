#ifndef SOCKET_DATAGRAMA_H_
#define SOCKET_DATAGRAMA_H_

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <cstring>
#include "PaqueteDatagrama.h"

using namespace std;

class SocketDatagrama
{
	public:
		SocketDatagrama(int);
		~SocketDatagrama();
		int recibe(PaqueteDatagrama &p);
		int recibeTimeout(PaqueteDatagrama &p, time_t segundos, suseconds_t microsegundos);
		int envia(PaqueteDatagrama &p);
		void setBroadcast();

	private:
		struct sockaddr_in direccionLocal;
		struct sockaddr_in direccionForanea;
		int s; // ID Socket
		struct timeval timeout;
};

#endif