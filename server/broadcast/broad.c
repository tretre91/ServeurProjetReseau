#include "broad.h"
#include "broadcast.h"
#include <stdlib.h>
#include <ifaddrs.h>

BroadcastSocket listener, sender;

int init_broadcast(const char* ip, int port) {
    listener = broadcastSocket(NULL, port, BROADCAST_RECEIVE);
    sender = broadcastSocket(ip, port, BROADCAST_SEND);

    return listener != NULL && sender != NULL;
}

int receive_from_broadcast(char* buffer, int size) {
    return receiveBroadcastMessage(listener, buffer, size);
}

int send_to_broadcast(const char* message, int size) {
    return sendBroadcastMessage(sender, message, size);
}

void close_broadcast() {
    closeBroadcastSocket(listener);
    closeBroadcastSocket(sender);
}