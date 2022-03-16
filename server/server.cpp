/*
 * inspiré de https://github.com/atwilc3000/sample/blob/master/Bluetooth/rfcomm_server.c
 */
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>

#include <array>
#include <fmt/core.h>
#include <string>

size_t read(int socket, std::string& buffer) {
    buffer.resize(1024);
    size_t bytes_read = recv(socket, buffer.data(), buffer.size(), 0);
    if (bytes_read != -1) {
        buffer.resize(bytes_read);
    }
    return bytes_read;
}

int main(int argc, char** argv) {
    const bdaddr_t bdaddr_any = {{0, 0, 0, 0, 0, 0}};
    sockaddr_rc local_address = {0};  /* local bluetooth adapter's info */
    sockaddr_rc client_address = {0}; /* filled in with remote (client) bluetooth device's info */
    std::string buffer(1024, '\0');

    fmt::print("Start Bluetooth RFCOMM server...\n");

    /* allocate socket */
    int server_sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    local_address.rc_family = AF_BLUETOOTH;       /* Addressing family, always AF_BLUETOOTH */
    bacpy(&local_address.rc_bdaddr, &bdaddr_any); /* Bluetooth address of local bluetooth adapter */
    local_address.rc_channel = 8;                 /* port number of local bluetooth adapter */

    fmt::print("binding\n");
    if (bind(server_sock, (sockaddr*)&local_address, sizeof(local_address)) < 0) {
        perror("failed to bind");
        exit(1);
    }

    fmt::print("listening\n");
    /* put socket into listening mode */
    listen(server_sock, 1); /* backlog is one */

    /* accept one connection */
    unsigned int opt = sizeof(client_address);
    int client_sock = accept(server_sock, (sockaddr*)&client_address, &opt); /* return new socket for connection with a client */

    ba2str(&client_address.rc_bdaddr, buffer.data());
    fmt::print("Connected from {}\n", std::string_view{buffer.data()});

    /* read data from the client */
    size_t bytes_sent;
    while (read(client_sock, buffer) != -1 && buffer != "close") {
        fmt::print("received '{}'\n", buffer);
        for (char& c : buffer) {
            c = std::toupper(c);
        }
        bytes_sent = send(client_sock, buffer.data(), buffer.size(), 0);
        fmt::print("Sent {} bytes to client: '{}'\n", bytes_sent, buffer);
    }

    /* close connection */
    close(client_sock);
    close(server_sock);
    return 0;
}
