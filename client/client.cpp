#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <cstdio>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <thread>
#include <fmt/core.h>
#include <iostream>

void send_loop(int socket) {
    std::string message;
    ssize_t bytes_written = 0;
    while (message != "stop" && bytes_written != -1) {
        std::getline(std::cin, message);
        bytes_written = send(socket, message.data(), message.size(), 0);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fmt::print("Usage: {} <adresse>\n", argv[0]);
        return 0;
    }

    const char* server_address = argv[1];

    int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (sock == -1) {
        fmt::print("Echec de la creation du socket\n");
        return -1;
    }

    sockaddr_rc addr = {0};
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = 1;
    str2ba(server_address, &addr.rc_bdaddr);

    fmt::print("Tentative de connexion au serveur (adresse : {}) ... ", server_address);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fmt::print("Echec de la connexion\n");
        close(sock);
        return -1;
    }
    
    fmt::print("Connecte\n");

    std::thread thr(&send_loop, sock);

    ssize_t bytes_read = 0;
    std::array<char, 1024> buffer;
    std::string_view answer;

    while (answer != "stop") {
        bytes_read = recv(sock, buffer.data(), buffer.size(), 0);
        if (bytes_read == -1) {
            break;
        }
        answer = std::string_view(buffer.data(), bytes_read);
        fmt::print("recu: {}\n", answer);
    }

    thr.join();
    close(sock);
    return 0;
}
