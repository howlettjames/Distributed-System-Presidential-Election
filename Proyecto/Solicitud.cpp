#include <iostream>
#include "Solicitud.h"

using namespace std;

Solicitud::Solicitud()
{
	requestId = 0;
	socketlocal = new SocketDatagrama(0);
}

Solicitud::~Solicitud()
{
	delete socketlocal;
} 

struct mensaje * Solicitud::doOperation(char *ip, int puerto, int operationId, char *arguments, int args_size)
{
	struct mensaje men;
	int n = -1, nreenvios = 7;

	// MENSAJE A ENVIAR AL SERVIDOR
	men.messageType = 0;										// 0 = Solicitud, 1 = Respuesta
	men.requestId = requestId++;								// ID de solicitud generado automáticamente
	men.operationId = operationId;								// ID de la operación deseada
	memcpy(men.arguments, arguments, args_size);				// Copiamos los argumentos pasados dentro del mensaje a enviar

	// CREAMOS UN DATAGRAMA CON EL PUERTO, DIRECCIÓN DEL SERVIDOR Y  EL MENSAJE A ENVIAR
	PaqueteDatagrama paquete((char *) &men, sizeof(struct mensaje), ip, puerto);

	// PREPARAMOS UN PAQUETE VACÍO DONDE RECIBIR LA RESPUESTA
	PaqueteDatagrama paquete1(sizeof(struct mensaje));
	while(n == -1 && nreenvios >= 0)
	{
		// ENVÍAMOS EL DATAGRAMA AL SERVIDOR PARA QUE HAGA LA OPERACIÓN CORRESPONDIENTE
		socketlocal->envia(paquete);

		// SOLO HAREMOS 7 REENVÍOS
		nreenvios--;

		// ESPERAMOS RECIBIR LA RESPUESTA DEL SERVIDOR CON UN TIMEOUT DE 2 SEGS
		n = socketlocal->recibeTimeout(paquete1, 5, 0);
		// SI LA FUNCIÓN REGRESA "-1" QUIERE DECIR QUE SE AGOTÓ EL TIMEOUT
	}

	if(nreenvios < 0)
	{
		fprintf(stderr, "El servidor no está disponible, límite de reenvíos alcanzado\n");
		exit(EXIT_FAILURE);
	}
	
	// OBTENEMOS EL MENSAJE ENVIADO POR EL SERVIDOR 
	struct mensaje *men_res = (struct mensaje *) paquete1.obtieneDatos();
	// cout << "Paquete recibido desde: " << paquete1.obtieneDireccion() << " : " << paquete1.obtienePuerto() << endl;
	
	return men_res;
}
