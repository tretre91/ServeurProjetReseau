#include "server.hpp"
#include <fmt/core.h>

#include "bluetooth.h"

int main(int argc, char** argv) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    fmt::print("Demarrage du serveur ... ");

    if (!init_service()) {
        fmt::print("Echec de la creation du service\n");
        return -1;
    }

    int server_socket = create_socket();
    if (server_socket == -1) {
        return -1;
    }

    fmt::print("Pret!\n");

    std::string ip;
    int port;
    if (argc > 2) {
        ip = argv[1];
        port = std::atoi(argv[2]);
    } else {
        ip = "192.168.0.255";
        port = 5000;
    }

    Server server(server_socket, ip, port);

    while (server.run()) {}

    close(server_socket);
    close_service();

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
