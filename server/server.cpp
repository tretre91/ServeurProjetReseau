#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <cstdio>
#include <sys/socket.h>
#include <unistd.h>

#include "server.hpp"
#include <fmt/core.h>
#include <string>

using rearm = dasynq::rearm;

Server::Server(int socket) : server_socket(socket) {
    // on ajoute une fonction lambda qui sera appelée lorsqu'une nouvelle connexion sera en attente
    // sur le socket du serveur
    loop_type::fd_watcher::add_watch(event_loop, server_socket, dasynq::IN_EVENTS, [&](loop_type& loop, int sock, int flags) {
        fmt::print("Nouvelle tentative de connexion ... ");

        sockaddr_rc client_address = {0};
        unsigned int addr_len;

        int client_socket = accept(sock, (sockaddr*)&client_address, &addr_len);

        if (client_socket == -1) {
            fmt::print("Echec de la connexion\n");
        } else {
            std::string buffer(18, '\0');
            ba2str(&client_address.rc_bdaddr, buffer.data());
            fmt::print("Succes (adresse : {})\n", buffer);

            Client* client = new Client(client_socket, *this);
            connected_clients++;
            client->add_watch(loop, client_socket, flags);
            clients.push_front(client);
        }

        return rearm::REARM;
    });
}

bool Server::run() {
    if (should_close) {
        return false;
    } else {
        event_loop.run();
        return true;
    }
}

void Server::send_to_all(std::string_view message) {
    for (auto it = clients.begin(); it != clients.end();) {
        if ((*it)->write(message) == -1) {
            auto current = it++;
            (*current)->deregister(event_loop);
        } else {
            ++it;
        }
    }
}

void Server::leave(Client* client) {
    connected_clients--;
    should_close = connected_clients == 0;
    clients.remove(client);
    delete client;
}

Client::Client(int socket, Server& chat_room) : socket(socket), server(chat_room) {}

rearm Client::fd_event(Server::loop_type&, int fd, int flags) {
    ssize_t byte_count = recv(fd, buffer.data(), buffer.size(), 0);

    if (byte_count == -1) {
        return rearm::REMOVE;
    } else {
        std::string_view message(buffer.data(), byte_count);
        fmt::print("Recu de {} : '{}'\n", fd, message);
        if (message == "stop") {
            write(message);
            close(fd);
            return rearm::REMOVE;
        } else {
            server.send_to_all(message);
        }
        return rearm::REARM;
    }
}

ssize_t Client::write(std::string_view message) {
    return send(socket, message.data(), message.size(), 0);
}

// void Client::watch_removed() noexcept {
//     chat.leave(this);
// }
