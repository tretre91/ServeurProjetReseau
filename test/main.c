#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "bluetooth-server/bluetooth_server.h"
#include "broadcast-messages/broadcast.h"

/*** Thread ***/

// créer un thread
int createThread(void* (*proc)(void* args), void* args) {
	pthread_t threadId;
	return pthread_create(&threadId, NULL, proc, args) == 0;
}

/*** Serveur bluetooth ***/

// serveur broadcast
BroadcastSocket broadcastServer;

// nombre max de clients en bluetooth
#define BLUETOOTH_CLIENT_MAX 16

// liste des clients connectés en bluetooth
BluetoothClient bluetoothClients[BLUETOOTH_CLIENT_MAX] = { 0 };

// mutex pour l'accès à la liste des clients bluetooth
pthread_mutex_t bluetoothClientsMutex = PTHREAD_MUTEX_INITIALIZER;

// ajouter un client bluetooth à la liste
int addBluetoothClient(BluetoothClient client) {
	
	int success = 0;
	
	pthread_mutex_lock(&bluetoothClientsMutex);
	
	for (int i = 0; i < BLUETOOTH_CLIENT_MAX; i++) {
		
		if (bluetoothClients[i] == 0) {
			bluetoothClients[i] = client;
			success = 1;
			break;
		}
	}
	
    pthread_mutex_unlock(&bluetoothClientsMutex);
    
    return success;
}

// retirer un client bluetooth de la liste
void removeBluetoothClient(BluetoothClient client) {
	
	pthread_mutex_lock(&bluetoothClientsMutex);
	
	for (int i = 0; i < BLUETOOTH_CLIENT_MAX; i++) {
		
		if (bluetoothClients[i] == client) {
			bluetoothClients[i] = 0;
			break;
		}
	}
	
    pthread_mutex_unlock(&bluetoothClientsMutex);
}

// envoyer un message à tous les clients en bluetooth
void sendMessageToBluetoothClients(const char* message) {
	
	pthread_mutex_lock(&bluetoothClientsMutex);
	
	for (int i = 0; i < BLUETOOTH_CLIENT_MAX; i++) {
		
		if (bluetoothClients[i] != 0) {
			sendString(bluetoothClients[i], message);
		}
	}
	
    pthread_mutex_unlock(&bluetoothClientsMutex);
}

// gérer les messages envoyés par un client en bluetooth
int processBluetoothClientMessages(BluetoothClient client) {
	
	char message[128];
	
	puts("info: connexion d'un client");
	
	if (!addBluetoothClient(client)) {
		puts("erreur: limite du nombre de clients atteinte");
		return 1;
	}
	
	while (receiveString(client, message, sizeof(message)) >= 0) {
		
		printf("message reçu en bluetooth: '%s'\n", message);		
		
		if (sendBroadcastMessage(broadcastServer, message) < 0) {
			break;
		}
	}
	
	puts("info: un client s'est déconnecté");
		
	return 1;
}

// serveur bluetooth
void* bluetoothServerProc(void* args) {
	
	puts("info: démarrage du serveur bluetooth");
	
	if (!bluetoothServer(1, &processBluetoothClientMessages, 1)) {
		puts("erreur: bluetoothServer()");
	}
	
	puts("info: arrêt du serveur bluetooth");
	
	return NULL;
}

// créer le thread sur lequel tourne le serveur bluetooth
int createBluetoothServerThread() {
	return createThread(&bluetoothServerProc, NULL);
}

/*** Broadcast ***/

// créer le serveur broadcast
int createBroadcastServer() {
	
	puts("info: création du serveur broadcast");
	
	char message[128];
	
	broadcastServer = broadcastSocket("192.168.1.255", 5000, BROADCAST_SEND | BROADCAST_RECEIVE);
	if (broadcastServer == NULL) {
		puts("erreur: broadcastSocket()");
		return 0;
	}
	
	while (receiveBroadcastMessage(broadcastServer, message, sizeof(message)) >= 0) {
		printf("message reçu en broadcast: '%s'\n", message);		
		sendMessageToBluetoothClients(message);
	}
	
	closeBroadcastSocket(broadcastServer);
	
	puts("info: arrêt du serveur broadcast");
	
	return 1;
}

/*** Main ***/

int main() {
	createBluetoothServerThread();
	createBroadcastServer();
	puts("appuyer sur Entrée pour terminer...");
	getchar();
	return 0;
}
