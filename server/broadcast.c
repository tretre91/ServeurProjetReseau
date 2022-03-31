#include "broadcast.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static void initAddr(struct sockaddr_in* addrIn, in_addr_t addr, int port) {
	memset(addrIn, 0, sizeof(struct sockaddr_in));
	addrIn->sin_family = AF_INET;
	addrIn->sin_addr.s_addr = addr;
	addrIn->sin_port = htons(port);
}

static int enableSocketOption(int socket, int option) {
	static const int socketOption = 1;
	return setsockopt(socket, SOL_SOCKET, option, &socketOption, sizeof(int)) >= 0;
}

BroadcastSocket broadcastSocket(const char* ip, int port, int option) {
	
	BroadcastSocket s = malloc(sizeof(struct _BroadcastSocket));
	if (s == NULL) {
		return NULL;
	}
	
	s->socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (s->socket < 0) {
		free(s);
		return NULL;
	}
	
	if (option & BROADCAST_SEND) {
		
		if (!enableSocketOption(s->socket, SO_BROADCAST)) {
			close(s->socket);
			free(s);
			return NULL;
		}
	
		initAddr(&s->sendAddr, inet_addr(ip), port);
	}
	
	if (option & BROADCAST_RECEIVE) {
		
		if (!enableSocketOption(s->socket, SO_REUSEADDR)) {
			close(s->socket);
			free(s);
			return NULL;
		}
		
		initAddr(&s->receiveAddr, INADDR_ANY, port);
		
		if (bind(s->socket, (struct sockaddr*)&s->receiveAddr, sizeof(struct sockaddr_in)) < 0) {
			close(s->socket);
			free(s);
			return NULL;
		}
	}
	
	return s;
}

int sendBroadcastMessage(BroadcastSocket s, const char* message, int size) {
	return sendto(s->socket, message, size, 0, (struct sockaddr*)&s->sendAddr, sizeof(struct sockaddr_in));
}

int receiveBroadcastMessage(BroadcastSocket s, char* message, int size) {
	socklen_t addrLength = sizeof(struct sockaddr_in);
	int n = recvfrom(s->socket, message, size - 1, 0, (struct sockaddr*)&s->receiveAddr, &addrLength);
	if (n >= 0) {
		message[n] = '\0';
	}
	return n;
}

void closeBroadcastSocket(BroadcastSocket s) {
	close(s->socket);
	free(s);
}