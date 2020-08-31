#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h> 	// Para struct timeval
#include <vector>  		// Vector
#include <algorithm> 	// sort()
#include <unistd.h>		// sleep()
#include <sstream>		// stringstream
#include <thread>

#include "Respuesta.h"
#include "Solicitud.h"
#include "registro.h"
#include "mongoose.h"

using namespace std;

// -------------------------------------------------- VARIABLES GLOBALES PARA SERIVDOR DE VOTOS -------------------------------------------
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
 
#define SERVIDOR_BROAD_PORT 9000 // Puerto donde cada servidor de votos recibirá mensajes de broadcast (en el constructor de la clase SocketDatagrama ponemos la opción de reuso de puerto, para pruebas locales)

#define SERVIDOR_PORT_1	7200
#define SERVIDOR_PORT_2	7201
#define SERVIDOR_PORT_3	7202

int num_server = 0;							// Variable recibida por línea de comandos para identificar el número de servidor
// -------------------------------------------------/ VARIABLES GLOBALES PARA SERIVDOR DE VOTOS -------------------------------------------

// -------------------------------------------------------- Á R B O L   T R I E ---------------------------------------------------------
const int ALPHABET_SIZE = 10;				// Tamaño del alfabeto que estaremos manejando en el Trie, en este caso los números 0-9.
// NODO DEL ÁRBOL TRIE
struct TrieNode 
{ 
    struct TrieNode *children[ALPHABET_SIZE]; 
    // isEndOfWord is true if the node represents end of a word 
    bool isEndOfWord; 
}; 

// FUNCIONES DEL ÁRBOL TRIE
struct TrieNode *getNode(void);
void insert(struct TrieNode *root, string key);
bool search(struct TrieNode *root, string key);
// -------------------------------------------------------/ Á R B O L   T R I E ---------------------------------------------------------

// ------------------------------------------------ VARIABLES GLOBALES PARA EL SERVIDOR WEB ---------------------------------------------
static const char *s_http_port_1 = "8000";              // Puerto en el que correrá nuestro servidor web    
static const char *s_http_port_2 = "8001";              // Puerto en el que correrá nuestro servidor web    
static const char *s_http_port_3 = "8002";              // Puerto en el que correrá nuestro servidor web    
static struct mg_serve_http_opts s_http_server_opts;	// Estructura para almacenar opciones HTTP de nuestro servidor

int num_servidores_a_esperar = 3;						// Para conocer el número de servidores activos(a esperar por una respuesta),
														// esta es una optimización, para que el servidor web que manda el mensaje broadcast
														// no tenga que esperar a que se agote el timeout.	
// -----------------------------------------------/ VARIABLES GLOBALES PARA EL SERVIDOR WEB ---------------------------------------------

// ----------------------------------------------------- S T A T I S T I C S   H A N D L E R ------------------------------------------------
// FUNCIÓN QUE MANEJA LA PETICIÓN DE ESTADÍSTICAS A CADA SERVIDOR
static void handle_search_services(struct mg_connection *nc, struct http_message *hm) 
{
    SocketDatagrama socket_datagrama(0);        // Abrimos un socket para enviar un mensaje broadcast
    socket_datagrama.setBroadcast();            // Habilitamos el envió de mensajes broadcast
    int codigo_operacion = 1;					// Mandamos un 1 por broadcast a todos los servidores para que manden el número de votos que llevan
    char servidor_json_1[512] = {0};            // Para almacenar en formato JSON la IP y número de votos registrados de cada servidor
    char servidor_json_2[512] = {0};            // Para almacenar en formato JSON la IP y número de votos registrados de cada servidor
    char servidor_json_3[512] = {0};            // Para almacenar en formato JSON la IP y número de votos registrados de cada servidor
    int *info_a_recibir;						// Para almacenar temporalmente el número de servidor y sus votos respectivamente
    bool servidorRespondio1, servidorRespondio2, servidorRespondio3;
    int num_servidores_respondieron = 0;		// Para ir contando el número de servidores que han respondido al broadcast
    bool disminuirNumServidoresAEsperar;			// Cuando un servidor manda un "-1" en el campo de votos, significa que esta cerrando comunicación

    // CREAMOS UN PAQUETE PARA SER ENVIADO BROADCAST A TODOS LOS SERVIDORES QUE ESTÉN DISPONIBLES EN EL PUERTO ESPECIFICADO
    PaqueteDatagrama paquete_broadcast((char *) &codigo_operacion, sizeof(int), "25.255.255.255", SERVIDOR_BROAD_PORT);
    // CREAMOS OTRO PAQUETE VACÍO, PARA ALMACENAR LOS PAQUETES ENVIADOS POR LOS SERVIDORES DISPONIBLES
    PaqueteDatagrama paquete_recepcion(3 * sizeof(int));

    sprintf(servidor_json_1, "{");
    sprintf(servidor_json_2, "{");
    sprintf(servidor_json_3, "{");
    
    // ENVIAMOS EL MENSAJE DE BROADCAST
    socket_datagrama.envia(paquete_broadcast);
    
    // printf("Esperando respuesta(s) al broadcast...\n");
    servidorRespondio1 = false;
    servidorRespondio2 = false;
    servidorRespondio3 = false;
    num_servidores_respondieron = 0;
    disminuirNumServidoresAEsperar = false;
    while(num_servidores_respondieron < num_servidores_a_esperar && socket_datagrama.recibeTimeout(paquete_recepcion, 5, 0) > 0)
    {
        // printf("--------------------------------------------------\n");
        // printf("IP: %s\n", paquete_recepcion.obtieneDireccion());
        // printf("Puerto: %d\n", paquete_recepcion.obtienePuerto());
        info_a_recibir = (int *) paquete_recepcion.obtieneDatos();
        // printf("Votos que lleva el servidor numero: %d - %d\n", info_a_recibir[0], info_a_recibir[1]);

        // SI EL SERVIDOR DEVUELVE UN "-1" SIGNIFICA QUE ESTA CERRANDO COMUNICACIÓN
    	if(info_a_recibir[2] == -1)
    		disminuirNumServidoresAEsperar = true;

    	// CHECAMOS EL NÚMERO DE SERVIDOR QUE ENVÍO LA RESPUESTA Y DE ACUERDO A ESO LLENAMOS LA CADENA A MANERA DE JSON
        if(info_a_recibir[0] == 1)
        {
        	sprintf(servidor_json_1 + strlen(servidor_json_1), " \"ip\": \"%s\", \"num_votos\": %d ", paquete_recepcion.obtieneDireccion(), info_a_recibir[1]);
        	servidorRespondio1 = true;
        }
        else if(info_a_recibir[0] == 2)
        {
        	sprintf(servidor_json_2 + strlen(servidor_json_2), " \"ip\": \"%s\", \"num_votos\": %d ", paquete_recepcion.obtieneDireccion(), info_a_recibir[1]);
        	servidorRespondio2 = true;
        }
        else if(info_a_recibir[0] == 3)
        {
        	sprintf(servidor_json_3 + strlen(servidor_json_3), " \"ip\": \"%s\", \"num_votos\": %d ", paquete_recepcion.obtieneDireccion(), info_a_recibir[1]);
        	servidorRespondio3 = true;
        }
        num_servidores_respondieron++;
    }
    // printf("--------------------------------------------------\n");
    // AQUÍ DISMINUIMOS EL NÚMERO DE SERVIDORES A ESPERAR PUES SI LO HACEMOS DENTRO DEL CICLO PODRÍA CAUSAR CONFLICTO
    if(disminuirNumServidoresAEsperar)
    	num_servidores_a_esperar--;

    // SI ALGÚN SERVIDOR NO RESPONDIÓ, SE LLENA LA CADENA EN FORMATO JSON CON LOS SIGUIENTES DATOS
    if(!servidorRespondio1)
    {
    	sprintf(servidor_json_1 + strlen(servidor_json_1), " \"ip\": \"None\", \"num_votos\": -1 ");
    	// printf("Servidor 1 no respondio\n");
    }
    if(!servidorRespondio2)
    {
    	sprintf(servidor_json_2 + strlen(servidor_json_2), " \"ip\": \"None\", \"num_votos\": -1 ");
    	// printf("Servidor 2 no respondio\n");
    }
    if(!servidorRespondio3)
    {
    	sprintf(servidor_json_3 + strlen(servidor_json_3), " \"ip\": \"None\", \"num_votos\": -1 ");
    	// printf("Servidor 3 no respondio\n");
    }
    
    sprintf(servidor_json_1 + strlen(servidor_json_1), "}");
    sprintf(servidor_json_2 + strlen(servidor_json_2), "}");
    sprintf(servidor_json_3 + strlen(servidor_json_3), "}");
    
    // printf("%s\n", servidor_json_1);
    // printf("%s\n", servidor_json_2);
    // printf("%s\n", servidor_json_3);

	// Use chunked encoding in order to avoid calculating Content-Length
	mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
	// ENVIAMOS DENTRO DE UN OBJETO JSON LOS TRES OBJETOS JSON CON LA INFORMACIÓN DE LOS 3 SERVIDORES
	mg_printf_http_chunk(nc, "{ \"servidor1\": %s, \"servidor2\": %s, \"servidor3\": %s }", servidor_json_1, servidor_json_2, servidor_json_3);
	// Send empty chunk, the end of response
	mg_send_http_chunk(nc, "", 0);
}
// ----------------------------------------------------/ S T A T I S T I C S   H A N D L E R ------------------------------------------------

// --------------------------------------------------------- E V E N T   M A N A G E R ------------------------------------------------
// ESTA FUNCIÓN SE ENCARGA DE MANEJAR CUALQUIER TIPO DE EVENTO QUE OCURRA EN NUESTRO SERVIDOR WEB,
// CADA EVENTO LEVANTA UNA BANDERA Y SE NOS PROPORCIONA LA INFORMACIÓN RELACIONADA A ESE EVENTO A TRAVÉS 
// DEL APUNTADOR "EV_DATA", EL CUAL DEBEMOS CASTEAR AL TIPO DE ESTRUCTURA DE DATOS DEPENDIENDO DEL EVENTO,
// EN ESTE CASO MANEJAREMOS EVENTOS HTTP, POR ESA RAZÓN OCUPAMOS LA ESTRUCTURA "HTTP_MESSAGE"
static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) 
{
    struct http_message *hm = (struct http_message *) ev_data;

    // CHECANDO QUE TIPO DE EVENTO OCURRIÓ
    switch(ev) 
    {
        case MG_EV_HTTP_REQUEST:
            // SI SE TRATA DE UN EVENTO HTTP, DEBEMOS CHECAR SI ES UNA PETICIÓN REST HACIA ALGÚN ENDPOINT e.g. "/statistics"
            // O SI SE ESTÁ PIDIENDO DEVOLVER ALGÚN ARCHIVO DENTRO DEL DIRECTORIO DEL SERVIDOR (ELSE) 
            if(mg_vcmp(&hm->uri, "/statistics") == 0) 
                handle_search_services(nc, hm);
            else
                mg_serve_http(nc, (struct http_message *) ev_data, s_http_server_opts);
        break;
        default:
        break;
    }
}
// --------------------------------------------------------/ E V E N T   M A N A G E R ------------------------------------------------

// --------------------------------------------------------- W E B   S E R V E R ------------------------------------------------------
// EL SERVIDOR WEB CORRE EN UN HILO QUE ES LANZADO POR EL SERVIDOR DE VOTOS
void funcion_mgr()
{
	struct mg_mgr mgr;                              // Manager de los eventos web
	struct mg_connection *nc;                       // Nuestra conexión web
    cs_stat_t st;          
    
    // INICIALIZANDO OBJETO MANEJADOR DE EVENTOS    
    mg_mgr_init(&mgr, NULL);                    
    // CREANDO CONEXIÓN DE ESCUCHA Y AÑADIÉNDOLA A LA FUNCIÓN "EV_HANDLER", EN EL PUERTO ESPECIFICADO  
    if(num_server == 1)
    	nc = mg_bind(&mgr, s_http_port_1, ev_handler);
    else if(num_server == 2)
    	nc = mg_bind(&mgr, s_http_port_2, ev_handler);
    else 
    	nc = mg_bind(&mgr, s_http_port_3, ev_handler);

    if(nc == NULL) 
    {
        fprintf(stderr, "Cannot bind to specified port\n");
        exit(1);
    }

    // INDICANDO QUE ESTA CONEXIÓN ACEPTA PROTOCOLO HTTP/WEBSOCKET
    mg_set_protocol_http_websocket(nc);
    // Set up HTTP server parameters
    s_http_server_opts.document_root = "web_root";  // Set up web root directory

    // CHECANDO QUE EL DIRECTORIO ROOT HAYA SIDO ESTABLECIDO CORRECTAMENTE
    if(mg_stat(s_http_server_opts.document_root, &st) != 0) 
    {
        fprintf(stderr, "%s", "Cannot find web_root directory, exiting\n");
        exit(1);
    }

	printf("Starting web server on port specified\n");
	// CHECANDO LOS EVENTOS QUE OCURREN EN NUESTRA CONEXIÓN WEB
	for(;;)
        mg_mgr_poll(&mgr, 500);
	
	mg_mgr_free(&mgr);
}
// ----------------------------------------------------------/ W E B   S E R V E R -------------------------------------------------------

// ----------------------------------------------------------- V O T O S   S E R V E R ---------------------------------------------------
int main(int argc, char const *argv[])
{
	// ================================== PARA CREACIÓN DE ARCHIVO A ESCRIBIR
	int registros_fd;								// File descriptor del archivo en donde escribiremos los registros
	stringstream stream;							// Var para creación de nombre de archivo a escribir
	// ================================== PARA FUNCIONAMIENTO DEL SERVIDOR DE VOTOS
	struct timeval timestamp;						// Estructura timeval permite almacenar la hora exacta
	struct TrieNode *root = getNode();				// Nodo raíz de nuestro árbol Trie
	struct registro_time registro_recibido;			// Para almacenar temporalmente los registros recibidos
	struct mensaje *mensaje_registro;				// Para almacenar cada mensaje de registro de parte del cliente
	Solicitud solicitud_timestamp;					// Creamos un objeto Solicitud para poder enviar nuestro timestamp a los demás servidores y recibir acuses de recibido de manera confiable.(PSR)
	struct mensaje *mensaje_timestamp;				// Para almacenar cada mensaje de timestmap de otros servidores
	struct mensaje *mensaje_acuse;					// Var para recibir acuses de los demás servidores
	struct timeval timestamp_server1;				// Var para recibir timestamp de server 1
	struct timeval timestamp_server2;				// Var para recibir timestamp de server 2
	char timestamps[sizeof(struct timeval) * 2];	// Var para mandar dos timestamps en un solo mensaje					
	char acuse[] = "Timestamp(s) recibido(s)";		// Var para acuse hacia el otro servidor
	char mensaje_salida[] = "Saliendo del grupo";	// Var para avisar al otro servidor que este servidor se sale del grupo
	char mensaje_only_one[] = "Only you";			// Var para avisar al otro servidor que es el único que queda
	bool isServer1Alive = true, isServer2Alive = true, isServer3Alive = true;				
	// ================================== PARA COMUNICACIÓN CON EL SERVIDOR WEB QUE MANDA MENSAJES BROADCAST
	SocketDatagrama socket_broadcast(SERVIDOR_BROAD_PORT);	// Creamos un socket para recibir mensaje broadcast
	socket_broadcast.setBroadcast();						// Habilitamos la opción de mensajes broadcast
	PaqueteDatagrama paquete_recepcion_broad(sizeof(int));  // Creamos un paquete para almacenar los mensajes de broadcast recibidos
	int info_a_enviar[3];									// Información de respuesta al broadcast, num_server y el número de votos
	int contador_votos = 0;									// Contados de los votos que lleva este servidor
	int codigo_operacion = 0;								// Código de operación que lleva el mensaje broadcast
	                             
	// CHECANDO QUE LA INVOCACIÓN SEA CORRECTA
	if(argc != 2)
	{
		printf("Forma de uso: %s <numero_servidor>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// CREANDO NOMBRE DEL ARCHIVO A ESCRIBIR DE ACUERDO A NÚMERO DE SERVIDOR
	num_server = atoi(argv[1]);
	stream << "servidor_registros" << num_server << ".dat";
	if((registros_fd = open(stream.str().c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0666)) == -1)
	{
		perror("servidor_registros.dat");
		exit(-1);
	}
	
	// CREAMOS UN OBJETO RESPUESTA PARA PODER RESPONDER CON UN TIMESTAMP A LAS SOLICITUDES DE REGISTRO DEL CLIENTE, DE MANERA CONFIABLE.(PSR)
	Respuesta* respuesta_cliente = new Respuesta(14670);
	// PROCEDIMIENTO NECESARIO PARA PODER OCUPAR LOS PUERTOS CORRESPONDIENTES. CREAMOS EL OBJETO INICIALIZÁNDOLO PARA QUE EL 
	// COMPILADOR NO MANDE UN ERROR, Y LUEGO LE REASIGNAMOS UN NUEVO PUERTO DEPENDIENDO DE QUE SERVIDOR ESTA CORRIENDO.
	delete respuesta_cliente;
	if(num_server == 1)
		respuesta_cliente = new Respuesta(SERVIDOR_PORT_1);
	else if(num_server == 2)
		respuesta_cliente = new Respuesta(SERVIDOR_PORT_2);
	else if(num_server == 3)
		respuesta_cliente = new Respuesta(SERVIDOR_PORT_3);
	else
	{
		printf("Solo se soportan hasta 3 servidores.\n");
		exit(EXIT_FAILURE);
	}

	// CREAMOS OTRO OBJETO RESPUESTA PARA PODER RESPONDER RECIBIR TIMESTMAPS DE OTROS SERVIDORES Y RESPONDER CON ACUSES, DE MANERA CONFIABLE. (PSR)	
	Respuesta* respuesta_server_timestamp = new Respuesta(15000);
	// PROCEDIMIENTO NECESARIO PARA PODER OCUPAR LOS PUERTOS CORRESPONDIENTES. CREAMOS EL OBJETO INICIALIZÁNDOLO PARA QUE EL 
	// COMPILADOR NO MANDE UN ERROR, Y LUEGO LE REASIGNAMOS UN NUEVO PUERTO DEPENDIENDO DE QUE SERVIDOR ESTA CORRIENDO. COMO EL SERVIDOR 1
	// ES EL QUE INICIA LA CADENA DE MENSAJES DE COMPARACIÓN DE TIMESTAMPS, NUNCA RECIBE TIMESTAMPS DE LOS DEMÁS SERVIDORES PARA COMPARAR,
	// POR ESO ES QUE EN SU CASO SIMPLEMENTE SE CREA EL OBJETO E INMEDIATAMENTE SE DESTRUYE.
	delete respuesta_server_timestamp;
	if(num_server == 2)
		respuesta_server_timestamp = new Respuesta(8201);
	else if(num_server == 3)
		respuesta_server_timestamp = new Respuesta(8202);
	
	// !!!!!!! = > IMPORTANTE <= !!!!!!!!	
	// EN ESTE PUNTO SE ECHA A ANDAR EL HILO QUE EJECUTARÁ EL SERVIDOR WEB, QUE ES QUIEN SE ENCARGARÁ DE ATENDER LAS SOLICITUDES HTTP
	thread hilo_web(funcion_mgr);

	// EL SERVIDOR DE VOTOS CORRE INDEFINIDAMENTE, ESCUCHANDO EN EL PUERTO CORRESPONDIENTE
	while(1)
	{
		// ESPERAMOS UN MENSAJE DE BROADCAST TRANSMITIDO POR CUALQUIERA DE LOS 3 SERVIDORES WEB, QUE PEDIRÁ EL NÚMERO DE VOTOS QUE LLEVA ESTE SERVIDOR
		// printf("-----------------------------------\n");
		// printf("Esperando mensaje broadcast...\n");
		if(socket_broadcast.recibeTimeout(paquete_recepcion_broad, 0, 1000) > 0)
		{
			codigo_operacion = *(int *) paquete_recepcion_broad.obtieneDatos();	
			// printf("Codigo de operacion recibido: %d\n", codigo_operacion);
			// SI EL CÓDIGO DE OPERACIÓN ES 1, EL SERVIDOR WEB NOS PIDE ENVIAR NUESTRO NÚMERO DE SERVER Y EL NÚMERO DE VOTOS REGISTRADOS	
			if(codigo_operacion == 1)
			{
				info_a_enviar[0] = num_server;
				info_a_enviar[1] = contador_votos;
                info_a_enviar[2] = 1;
				PaqueteDatagrama paquete_respuesta((char *) info_a_enviar, sizeof(int) * 3, paquete_recepcion_broad.obtieneDireccion(), paquete_recepcion_broad.obtienePuerto());
				socket_broadcast.envia(paquete_respuesta);
			}
		}
		// printf("-----------------------------------\n");
		
		// GUARDAMOS LA REFERENCIA AL MENSAJE RECIBIDO
		// printf("Esperando solicitud de cliente...\n");
		mensaje_registro = respuesta_cliente->getRequest();

		// IMPRIMIMOS LA INFORMACIÓN RECIBIDA 
		// cout << "SOLICITUD RECIBIDA" << endl;
		// cout << "Tipo de mensaje: " << mensaje_registro->messageType << endl;
		// cout << "ID de solicitud: " << mensaje_registro->requestId << endl;
		// cout << "ID de operación: " << mensaje_registro->operationId << endl;
		
		// EJECUTAMOS LA OPERACIÓN SOLICITADA
		if(mensaje_registro->operationId == guardar_registro)
		{
			// COPIAMOS EL REGISTRO EN LA VAR REGISTRO_RECIBIDO
			memcpy(&registro_recibido, mensaje_registro->arguments, sizeof(struct registro));

			// CREANDO UNA INSTANCIA DE OBJETO STRING A PARTIR DE LA CADENA ESTILO C CLÁSICA, EN ESTE CASO DEL CELULAR DE QUIEN SE HIZO EL REGISTRO
			string celularx(registro_recibido.celular);

			// HACEMOS LA BÚSQUEDA DE ESTE CELULAR SOBRE NUESTRO ÁRBOL TRIE
			if(search(root, celularx))
			{
				cout << "Ya existía este celular: " << celularx << endl;
				// sleep(3);
				// EN CASO DE EXISTIR PREVIAMENTE EL CELULAR EN NUESTRA BD, REGRESAMOS UN TIMSTAMP CON SEGUNDOS Y MICROSEGUNDOS IGUALES A 0
				registro_recibido.timestamp.tv_sec = 0;
				registro_recibido.timestamp.tv_usec = 0;
			}
			else
			{
				// INSERTAMOS EL CELULAR EN NUESTRO ÁRBOL TRIE
				insert(root, celularx);
				// OBTENIENDO LA HORA EXACTA ACTUAL, PARA GUARDAR EL REGISTRO JUNTO CON LA HORA EN QUE FUE ALMACENADO EN LA BD
				gettimeofday(&registro_recibido.timestamp, NULL);

				if(num_server == 1)
				{
					if(isServer2Alive)
					{
						// ENVIAMOS NUESTRO TIMESTAMP AL SERVIDOR ||2|| PARA QUE VERIFIQUE, ANTES DE RESPONDER, QUE NO SE REPITE EL MISMO TIMESTAMP
						mensaje_acuse = solicitud_timestamp.doOperation(SERVIDOR_IP_2, 8201, timestamp_, (char *) &registro_recibido.timestamp, sizeof(struct timeval));				
						// printf("1: Server 2 dice: %s\n", mensaje_acuse->arguments);
						if(mensaje_acuse->arguments[0] == 'S')
						{
							// printf("1: Cambiando a server 3\n");
							isServer2Alive = false;
							solicitud_timestamp.requestId--;	// Hacemos esto para mantener consistencia de los IDs de solicitud, pues si el servidor 2 
																// cerró comunicación, debemos enviar ahora el mismo timestamp, pero ahora al servidor 3
						}
						else if(mensaje_acuse->arguments[0] == 'O')
						{
							isServer2Alive = false;
							isServer3Alive = false;
							// printf("1: Servidor 2 y 3 fuera\n");	
						}
					}
					if(!isServer2Alive)
					{
						if(isServer3Alive)
						{
							// ENVIAMOS NUESTRO TIMESTAMP AL SERVIDOR ||3|| PARA QUE VERIFIQUE, ANTES DE RESPONDER, QUE NO SE REPITE EL MISMO TIMESTAMP
							memcpy(timestamps, &registro_recibido.timestamp, sizeof(struct timeval));
							memcpy(timestamps + sizeof(struct timeval), &registro_recibido.timestamp, sizeof(struct timeval));
							// EN EL CASO QUE EL SERVIDOR 3 TAMBIÉN MUERA, EL SERVIDOR 1 SE QUEDARÁ ESPERANDO HASTA QUE EL LÍMITE DE TIEMPO SE ACABE
							mensaje_acuse = solicitud_timestamp.doOperation(SERVIDOR_IP_3, 8202, timestamp_, timestamps, sizeof(struct timeval) * 2);
							// printf("1: Server 3 dice: %s\n", mensaje_acuse->arguments);
							if(mensaje_acuse->arguments[0] == 'S')
							{
								isServer3Alive = false;
								// printf("1: Servidor 2 y 3 fuera\n");
							}
						}
					}
				}
				else if(num_server == 2)
				{
					// RECIBIMOS EL TIMESTAMP DEL SERVIDOR 1
					mensaje_timestamp = respuesta_server_timestamp->getRequest();
					memcpy(&timestamp_server1, mensaje_timestamp->arguments, sizeof(struct timeval));
					// printf("2: Recibido timestamp de server 1: %ld : %ld\n", timestamp_server1.tv_sec, timestamp_server1.tv_usec);

					// ENVIANDO ACUSE A SERVIDOR 1
					respuesta_server_timestamp->sendReply(acuse, strlen(acuse));
					
					// CHECAMOS QUE NUESTRO TIMESTAMP(SERVER 2) NO SEA IGUAL AL DEL SERVER 1	
					if(timestamp_server1.tv_usec == registro_recibido.timestamp.tv_usec)
						registro_recibido.timestamp.tv_usec++;	

					if(isServer3Alive)
					{
						// ENVIAMOS NUESTRO TIMESTAMP Y EL DEL SERVER 1 AL SERVIDOR 3 PARA QUE VERIFIQUE, ANTES DE RESPONDER, QUE NO SE REPITE EL MISMO TIMESTAMP
						memcpy(timestamps, &timestamp_server1, sizeof(struct timeval));
						memcpy(timestamps + sizeof(struct timeval), &registro_recibido.timestamp, sizeof(struct timeval));
						mensaje_acuse = solicitud_timestamp.doOperation(SERVIDOR_IP_3, 8202, timestamp_, timestamps, sizeof(struct timeval) * 2);
						// printf("2: Server 3 dice: %s\n", mensaje_acuse->arguments);
						if(mensaje_acuse->arguments[0] == 'S')
						{
							isServer3Alive = false;
							// printf("2: Cambiando solo a servidor 1\n");
						}
					}
				}
				else if(num_server == 3)
				{
					// RECIBIMOS EL TIMESTAMP DEL SERVIDOR 1 Y 2, O PUEDE QUE SOLO DEL SERVIDOR 1 O DEL 2, EN CUYO CASO RECIBIREMOS EL SEGUNDO 
					// TIMESTAMP IDÉNTICO AL PRIMERO
					mensaje_timestamp = respuesta_server_timestamp->getRequest();
					memcpy(&timestamp_server1, mensaje_timestamp->arguments, sizeof(struct timeval));
					memcpy(&timestamp_server2, mensaje_timestamp->arguments + sizeof(struct timeval), sizeof(struct timeval));
					// printf("3: Recibido timestamp de server 1: %ld : %ld\n", timestamp_server1.tv_sec, timestamp_server1.tv_usec);
					// printf("3: Recibido timestamp de server 2: %ld : %ld\n", timestamp_server2.tv_sec, timestamp_server2.tv_usec);

					// ENVIANDO ACUSE AL SERVIDOR 2 O 1, DEPENDE QUIEN HAYA MANDADO LOS TIMESTAMPS
					respuesta_server_timestamp->sendReply(acuse, strlen(acuse));
					
					// CHECANDO QUE NUESTRO TIMESTAMP(SERVER 3) NO SEA IGUAL AL DEL SERVIDOR 1 O 2
					if(timestamp_server1.tv_usec == registro_recibido.timestamp.tv_usec)
						registro_recibido.timestamp.tv_usec++;	
					if(timestamp_server2.tv_usec == registro_recibido.timestamp.tv_usec)
						registro_recibido.timestamp.tv_usec++;	
				}

				// ESCRIBIENDO EL REGISTRO EN EL ARCHIVO INDICADO POR LÍNEA DE COMANDOS
				write(registros_fd, &registro_recibido, sizeof(struct registro_time));
				// HACEMOS FSYNC() PARA QUE EL REGISTRO SEA ESCRITO INMEDIATAMENTE DE RAM A DISCO
				fsync(registros_fd);
				// ENVIAMOS LA RESPUESTA AL CLIENTE
				respuesta_cliente->sendReply((char *) &registro_recibido.timestamp, sizeof(struct timeval));
				contador_votos++;

				// IMPRIMIMOS LA INFORMACIÓN DEL REGISTRO RECIBIDO
				// printf("Cel: %s || CURP: %s || Partido: %s\n\n", registro_recibido.celular, registro_recibido.CURP, registro_recibido.partido);
				// printf("Segundos : Microsegundos -> %ld : %ld\n", registro_recibido.timestamp.tv_sec, registro_recibido.timestamp.tv_usec);
			}
		}
		else if(mensaje_registro->operationId == cerrando_comunicacion)
		{
			// ESPERAMOS UN MENSAJE DE BROADCAST TRANSMITIDO POR CUALQUIERA DE LOS 3 SERVIDORES WEB, QUE PEDIRÁ EL NÚMERO DE VOTOS QUE LLEVA ESTE SERVER.
			// EN ESTE CASO, EL HILO DE CLIENTE HA CERRADO LA COMUNICACIÓN PUES HA ACABADO DE ENVIAR SUS REGISTROS CORRESPONDIENTES, POR ELLO
			// RESPONDEREMOS CON "-1" EN EL CAMPO DE VOTOS, Y ESPERAMOS NO 1000us SINO HASTA QUE NOS LLEGUE OTRO MENSAJE DE BROADCAST, PARA PODER 
			// AVISARLE APROPIADAMENTE AL SERVIDOR WEB CORRESPONDIENTE, QUE YA ESTAMOS FUERA DEL GRUPO DE SERVIDORES DE VOTOS. Y NO NOS ESPERE A CONTESTAR.
			// printf("Esperando mensaje broadcast para avisar salida...\n");
			if(socket_broadcast.recibe(paquete_recepcion_broad) > 0)
			{
				info_a_enviar[0] = num_server;
				info_a_enviar[1] = contador_votos;
                info_a_enviar[2] = -1;
				PaqueteDatagrama paquete_respuesta((char *) info_a_enviar, sizeof(int) * 3, paquete_recepcion_broad.obtieneDireccion(), paquete_recepcion_broad.obtienePuerto());
				socket_broadcast.envia(paquete_respuesta);
			}
		
			if(num_server == 2)
			{
				// AVISANDO A SERVIDOR 1 QUE SERVIDOR 2 ESTA FUERA
				mensaje_timestamp = respuesta_server_timestamp->getRequest();
				if(isServer3Alive)
					respuesta_server_timestamp->sendReply(mensaje_salida, strlen(mensaje_salida));	
				else
					respuesta_server_timestamp->sendReply(mensaje_only_one, strlen(mensaje_only_one));	
			}
			else if(num_server == 3)
			{
				// AVISANDO A SERVIDOR 2 QUE SERVIDOR 3 ESTA FUERA
				mensaje_timestamp = respuesta_server_timestamp->getRequest();
				respuesta_server_timestamp->sendReply(mensaje_salida, strlen(mensaje_salida));
			}

			printf("%d: terminando ejecucion...\n", num_server);
			// ENVIANDO LO QUE SEA AL CLIENTE, PARA QUE DEJE DE ESPERAR Y TERMINE SU HILO
			respuesta_cliente->sendReply((char *) &registro_recibido.timestamp, sizeof(struct timeval));
			// CERRANDO EL ARCHIVO DE ESCRITURA, DESTRUYENDO EL OBJETO DE RESPUESTA Y SALIENDO
			delete respuesta_cliente;
			close(registros_fd);
			exit(EXIT_SUCCESS);
		}
		else if(mensaje_registro->operationId == 0)
		{
			// SI EL ID DE OPERACIÓN ES CERO, EL SERVIDOR NO EFECTUARÁ NINGUNA OPERACIÓN, SE DESCARTA LA SOLICITUD REPETIDA
			continue;	
		}
	}
	return 0;
}
// ----------------------------------------------------------/ V O T O S   S E R V E R ----------------------------------------------------

// ------------------------------------------------------- F U N C I O N E S   T R I E ----------------------------------------------------
// Returns new trie node (initialized to NULLs) 
struct TrieNode *getNode(void) 
{ 
    struct TrieNode *pNode =  new TrieNode; 
  
    pNode->isEndOfWord = false; 
  
    for (int i = 0; i < ALPHABET_SIZE; i++) 
        pNode->children[i] = NULL; 
  
    return pNode; 
} 
  
// If not present, inserts key into trie 
// If the key is prefix of trie node, just 
// marks leaf node 
void insert(struct TrieNode *root, string key) 
{ 
    struct TrieNode *pCrawl = root; 
  
    for (int i = 0; i < key.length(); i++) 
    { 
        int index = key[i] - '0'; 
        if (!pCrawl->children[index]) 
            pCrawl->children[index] = getNode(); 
  
        pCrawl = pCrawl->children[index]; 
    } 
  
    // mark last node as leaf 
    pCrawl->isEndOfWord = true; 
} 
  
// Returns true if key presents in trie, else 
// false 
bool search(struct TrieNode *root, string key) 
{ 
    struct TrieNode *pCrawl = root; 
  
    for (int i = 0; i < key.length(); i++) 
    { 
        int index = key[i] - '0'; 
        if (!pCrawl->children[index]) 
            return false; 
  
        pCrawl = pCrawl->children[index]; 
    } 
  
    return (pCrawl != NULL && pCrawl->isEndOfWord); 
}
// ------------------------------------------------------/ F U N C I O N E S   T R I E ----------------------------------------------------
