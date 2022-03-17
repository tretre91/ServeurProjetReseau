#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// https://github.com/bitsbyte/bluetooth-programming-in-c/blob/master/rfcomm-server.c

int createServer(uint8_t channel) {
    int s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (s < 0) {
        return -1;
    }

    struct sockaddr_rc loc_addr = {0};
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = channel;

    int b = bind(s, (struct sockaddr*)&loc_addr, sizeof(loc_addr));
    if (b < 0) {
        close(s);
        return -1;
    }

    int l = listen(s, 1);
    if (l < 0) {
        close(s);
        return -1;
    }

    return s;
}

int waitForClient(int server) {
    return accept(server, NULL, NULL);
}

int waitForMessage(int client, char* message, int size) {
    int n = read(client, message, size - 1);
    if (n <= 0) {
        message[0] = '\0';
        return 0;
    }
    message[n] = '\0';
    return 1;
}

void sendMessage(int client, char* message) {
    write(client, message, strlen(message));
}

enum ProcessMessageCode
{
    CONTINUE,
    STOP_CONNECTION,
    STOP_SERVER
};

int processMessage(int client, char* message) {
    if (strcmp(message, "stop") == 0) {
        return STOP_SERVER;
    }

    printf("message: %s\n", message);
    sendMessage(client, "bien recu");

    return CONTINUE;
}

int main() {
    char message[512] = {'\0'};
    int end = 0;

    int server = createServer(1);
    if (server < 0) {
        puts("erreur: createServer()");
        return 1;
    }

    while (!end) {
        int client = waitForClient(server);
        if (client < 0) {
            puts("erreur: waitForClient()");
            continue;
        }

        puts("client connecte");

        while (waitForMessage(client, message, sizeof(message))) {
            int ret = processMessage(client, message);

            if (ret == STOP_CONNECTION) {
                break;
            }

            else if (ret == STOP_SERVER)
            {
                end = 1;
                break;
            }
        }

        puts("client deconnecte");

        close(client);
    }

    close(server);

    return 0;
}
