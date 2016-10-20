/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_NETWORK

#include <la_network.h>

#include <la.h>
#include <la_memory.h>

/**
 * Initialize Lib-Aldaron-Comm
 * @param sockets: Number of sockets to allocate.
**/
aldc_t* aldc_init(uint8_t sockets) {
	aldc_t* aldc = la_memory_allocate(sizeof(aldc_t));
	aldc->sockets = la_memory_allocate(sizeof(aldc_socket_t) * sockets);
	SDLNet_Init();
	aldc->set = SDLNet_AllocSocketSet(sockets);
	aldc->numSockets = 0;
	return aldc;
}

/**
 * Open a tcp client connection.
 * @param aldc: The library context.
 * @param socket: Pointer to uninit'd socket variable.
 * @param hostname: What to connect to.
 * @param port: Port to connect on.
**/
void aldc_tcp(aldc_t* aldc, aldc_tcp_socket_t* socket, const char* hostname,
	uint16_t port, size_t size)
{
	IPaddress serverIP;

	socket->socket = NULL;
	SDLNet_ResolveHost(&serverIP, hostname, port);
	if(serverIP.host == INADDR_NONE)
		la_panic("Couldn't resolve hostname %s.\n", hostname);
	socket->socket = SDLNet_TCP_Open(&serverIP);
	if(socket->socket == NULL)
		la_panic("Couldn't connect.\n");
	SDLNet_TCP_AddSocket(aldc->set, socket->socket);

	aldc->sockets[aldc->numSockets].socket = socket->socket;
	aldc->sockets[aldc->numSockets].size = size;
	aldc->numSockets++;
}

/**
 * Open a udp client connection
 * @param socket: Pointer to uninit'd socket variable.
 * @param port: Port to connect to.
**/
uint8_t aldc_udp(aldc_t* aldc, aldc_udp_socket_t* socket, uint32_t port,
	size_t size)
{
	socket->socket = SDLNet_UDP_Open(port);
	if(socket->socket == NULL)
		return 1;
	SDLNet_UDP_AddSocket(aldc->set, socket->socket);
	aldc->sockets[aldc->numSockets].socket = socket->socket;
	aldc->sockets[aldc->numSockets].size = size;
	aldc->numSockets++;
	return 0;
}

/**
 * Get packets from server.
 * @param aldc: The library context.
 * @param data: Data structure to output any found data.
 * @returns 1 if something is written to data.
 * @returns 0 if there are no new packets.
**/
uint8_t aldc_pull(aldc_t* aldc, void* data) {
	int i;

	SDLNet_CheckSockets(aldc->set, 0);
	for(i = 0; i < aldc->numSockets; i++) {
		if (SDLNet_SocketReady(aldc->sockets[i].socket)) {
			if(!SDLNet_TCP_Recv(aldc->sockets[i].socket,
				(char *)data, aldc->sockets[i].size))
				la_panic("connection lost.\n");
			else
				return 1;
		}
	}
	return 0;
}

/**
 * Kill Lib-Aldaron-Comm
**/
void aldc_kill(void) {
	SDLNet_Quit();
}

#endif
