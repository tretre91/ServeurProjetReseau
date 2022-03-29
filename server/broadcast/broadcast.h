#ifndef _BROADCAST_H_
#define _BROADCAST_H_

typedef struct BroadcastSocket* BroadcastSocket;

enum BroadcastSocketOption {
	BROADCAST_SEND = 1,
	BROADCAST_RECEIVE = 2
};

BroadcastSocket broadcastSocket(const char* ip, int port, int option);

int sendBroadcastMessage(BroadcastSocket s, const char* message, int size);

int receiveBroadcastMessage(BroadcastSocket s, char* message, int size);

void closeBroadcastSocket(BroadcastSocket s);

#endif