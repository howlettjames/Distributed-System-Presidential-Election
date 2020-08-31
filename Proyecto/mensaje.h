#ifndef MENSAJE_H_
#define MENSAJE_H_

#define TAM_MAX_DATA 500
// #define TAM_MAX_DATA 16
// #define TAM_MAX_DATA 60000

// OPERACIONES PERMITIDAS
#define guardar_registro 1
#define cerrando_comunicacion 2
#define timestamp_ 8
#define mensaje_repetido 9

// FORMATO DE CADA MENSAJE
struct mensaje
{
	int messageType;			// 0=Solicitud, 1=Respuesta
	unsigned int requestId;		//Identificador de mensaje
	int operationId;			//Identificador de la operacion
	char arguments[TAM_MAX_DATA];
};

#endif