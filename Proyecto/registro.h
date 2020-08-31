#ifndef REGISTRO_H_
#define REGISTRO_H_

#include <sys/time.h>
#include <time.h>

// FORMATO DE CADA REGISTRO
struct registro
{
	char celular[11];
	char CURP[19];
	char partido[4];
};

struct registro_time
{
	char celular[11];
	char CURP[19];
	char partido[4];
	struct timeval timestamp;
};

#endif