#include "server.hpp"
#include <fmt/core.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <sys/socket.h>

int main(int argc, char** argv) {
    const bdaddr_t bdaddr_any = {{0, 0, 0, 0, 0, 0}}; // BADDR_ANY est pas valide en c++ :(
    fmt::print("Demmarrage du serveur ... ");

    int server_socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (server_socket == -1) {
        return -1;
    }

    sockaddr_rc local_address = {0};
    local_address.rc_family = AF_BLUETOOTH;
    bacpy(&local_address.rc_bdaddr, &bdaddr_any);
    local_address.rc_channel = 1;

    if (bind(server_socket, (sockaddr*)&local_address, sizeof(local_address)) < 0) {
        fmt::print("Echec de bind");
        close(server_socket);
        return -1;
    }

    if (listen(server_socket, 10) == -1) {
        perror("Echec de listen");
        close(server_socket);
        return -1;
    }

    fmt::print("Pret!\n");

    ChatRoom::loop_type event_loop;

    ChatRoom chat(server_socket, event_loop);

    while (!chat.should_stop()) {
        event_loop.run();
    }

    close(server_socket);
    return 0;
}
