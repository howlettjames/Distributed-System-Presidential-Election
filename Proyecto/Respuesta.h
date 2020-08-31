#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include <iostream>
#include "SocketDatagrama.h"
#include "mensaje.h"

class Respuesta
{
	public:
		Respuesta();
		Respuesta(int pl);
		~Respuesta();
		struct mensaje *getRequest(void);
		void sendReply(char *respuesta, int tamanio_respuesta);
		unsigned int requestId;

	private:
		SocketDatagrama *socketlocal;
		struct mensaje message_recv;
		char ip[16];
		int puerto;
};

#endif