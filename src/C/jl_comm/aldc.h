#include "jl.h"
#include "SDL_net.h"

typedef struct {
	TCPsocket socket;
} aldc_tcp_socket_t;

typedef struct {
	UDPsocket socket;
} aldc_udp_socket_t;

typedef struct {
	void* socket;
	uint8_t socketIsTCP;
	size_t size;
} aldc_socket_t;

typedef struct {
	jl_t* jl;
	SDLNet_SocketSet set;
	uint8_t numSockets;
	aldc_socket_t* sockets;
} aldc_t;
