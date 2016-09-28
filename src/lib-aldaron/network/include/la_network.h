/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

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
	SDLNet_SocketSet set;
	uint8_t numSockets;
	aldc_socket_t* sockets;
} aldc_t;
