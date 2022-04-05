#ifndef _BROADCAST_H_
#define _BROADCAST_H_

#include <netinet/in.h>

typedef struct _BroadcastSocket* BroadcastSocket;

struct _BroadcastSocket {
	int socket;
	struct sockaddr_in sendAddr;
	struct sockaddr_in receiveAddr;
};

enum BroadcastSocketOption {
	BROADCAST_SEND = 1,
	BROADCAST_RECEIVE = 2
};

/**
 * @brief Crée un socket utilisé pour communiquer sur une adresse de diffusion
 */
BroadcastSocket broadcastSocket(const char* ip, int port, int option);

int sendBroadcastMessage(BroadcastSocket s, const char* message, int size);

int receiveBroadcastMessage(BroadcastSocket s, char* message, int size);

void closeBroadcastSocket(BroadcastSocket s);

#endif