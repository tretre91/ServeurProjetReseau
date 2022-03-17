/*
 * inspiré de https://github.com/atwilc3000/sample/blob/master/Bluetooth/rfcomm_client.c
 */

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <cstdio>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <fmt/core.h>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        fmt::print("Usage: {} <address>\n", argv[0]);
        return 0;
    }

    const char* server_address = argv[1];
    fmt::print("Start Bluetooth RFCOMM client, server addr {}\n", server_address);

    /* allocate a socket */
    int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    /* set the outgoing connection parameters, server's address and port number */
    sockaddr_rc addr = {0};
    addr.rc_family = AF_BLUETOOTH;           /* Addressing family, always AF_BLUETOOTH */
    addr.rc_channel = 1;                     /* server's port number */
    str2ba(server_address, &addr.rc_bdaddr); /* server's Bluetooth Address */

    /* connect to server with destination address and port */
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("failed to connect");
        exit(1);
    }
    fmt::print("connected...\n");

    /* send a message */
    size_t bytes_read;
    std::string msg;
    std::array<char, 1024> buffer;
    std::string_view answer;

    do {
        std::getline(std::cin, msg);
        send(sock, msg.data(), msg.size(), 0);

        bytes_read = recv(sock, buffer.data(), buffer.size(), 0);
        if (bytes_read == -1) {
            break;
        } else {
            answer = std::string_view(buffer.data(), bytes_read);
            fmt::print("Received '{}'\n", answer);
        }
    } while (answer != "CLOSE");

    close(sock);
    return 0;
}
