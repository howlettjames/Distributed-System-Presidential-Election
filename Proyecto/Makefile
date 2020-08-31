cliente: cliente.cpp Solicitud.o
	g++ cliente.cpp Solicitud.o SocketDatagrama.o PaqueteDatagrama.o -o cliente -lpthread -std=c++11
servidor: servidor.cpp mongoose.c Respuesta.o Solicitud.o
	g++ servidor.cpp mongoose.c Respuesta.o Solicitud.o SocketDatagrama.o PaqueteDatagrama.o -o servidor -lpthread -std=c++11
Respuesta.o: Respuesta.cpp SocketDatagrama.o Respuesta.h mensaje.h
	g++ Respuesta.cpp -c -std=c++11
Solicitud.o: Solicitud.cpp SocketDatagrama.o Solicitud.h mensaje.h
	g++ Solicitud.cpp -c -std=c++11
SocketDatagrama.o: SocketDatagrama.cpp PaqueteDatagrama.o SocketDatagrama.h
	g++ SocketDatagrama.cpp -c -std=c++11
PaqueteDatagrama.o: PaqueteDatagrama.cpp PaqueteDatagrama.h
	g++ PaqueteDatagrama.cpp -c -std=c++11

clean:
	rm -f *.o $(PROYECTO)
