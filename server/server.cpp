#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <cstdio>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <unistd.h>

#include "broad.h"
#include "client.hpp"
#include "server.hpp"

#include <fmt/core.h>
#include <string>
#include <thread>

using rearm = dasynq::rearm;

Server::Server(int socket, std::string_view broadcast_ip, int broadcast_port) : server_socket(socket) {
    fmt::print("Id du serveur : {}\n", get_id());

    if (!init_broadcast(broadcast_ip.data(), broadcast_port)) {
        fmt::print(stderr, "Echec lors de la création des sockets de broadcast\n");
        should_close = true;
        return;
    }

    server_intercomm = std::thread([&]() {
        int size = 0;
        constexpr size_t buffer_size = 1024;
        std::array<char, buffer_size> buffer;
        msg::CSMessage client_message;
        msg::SSMessage message;

        while (true) {
            size = receive_from_broadcast(buffer.data(), buffer.size());
            if (size > 0) {
                fmt::print("Received {} bytes\n", size);
                if (!message.ParseFromArray(buffer.data(), size)) {
                    fmt::print(stderr, "Failed to parse a broadcast message\n");
                    continue;
                }

                if (message.serverid() != m_id && message.type() == msg::SSMessage::CHAT) {
                    fmt::print("Recu de {}:{} : '{}'\n", message.serverid(), message.clientid(), message.data());
                    client_message.set_clientid(message.clientid());
                    client_message.set_data(message.data());
                    send_to_all_clients(client_message);
                }
            } else if (size == 0) {
                fmt::print(stderr, "Déconnexion du socket de diffusion\n");
                break;
            } else if (size == -1) {
                fmt::print(stderr, "Erreur lors de la lecture du socket de diffusion\n");
                break;
            }
        }
    });

    // on ajoute une fonction lambda qui sera appelée lorsqu'une nouvelle connexion sera en attente
    // sur le socket du serveur
    loop_type::fd_watcher::add_watch(event_loop, server_socket, dasynq::IN_EVENTS, [&](loop_type& loop, int sock, int flags) {
        fmt::print("Nouvelle tentative de connexion ... ");

        sockaddr_rc client_address = {0};
        unsigned int addr_len = sizeof(sockaddr_rc);

        int client_socket = accept(sock, (sockaddr*)&client_address, &addr_len);

        if (client_socket == -1) {
            fmt::print("Echec de la connexion\n");
        } else {
            std::string address(18, '\0');
            ba2str(&client_address.rc_bdaddr, address.data());
            fmt::print("Succes (adresse : {})\n", address);

            auto it = client_ids.find(address);
            uint8_t client_id;
            if (it == client_ids.end()) {
                client_id = ids_sequence++;
                client_ids[address] = client_id;
            } else {
                client_id = client_ids[address];
            }

            Client* client = new Client(client_socket, client_id, *this);
            connected_clients++;
            client->add_watch(loop, client_socket, flags);
            clients.push_front(client);
        }

        return rearm::REARM;
    });
}

bool Server::run() {
    if (should_close) {
        close_broadcast();
        server_intercomm.join();
        return false;
    } else {
        event_loop.run();
        return true;
    }
}

void Server::send_to_all_clients(const msg::CSMessage& message) {
    for (auto it = clients.begin(); it != clients.end();) {
        if ((*it)->id() != message.clientid() && (*it)->write(message) == -1) {
            auto current = it++;
            (*current)->deregister(event_loop);
        } else {
            ++it;
        }
    }
}

void Server::send_to_all(const msg::CSMessage& message) {
    static msg::SSMessage broadcast_message;
    static std::string buffer;
    broadcast_message.set_serverid(m_id);
    broadcast_message.set_clientid(message.clientid());
    broadcast_message.set_data(message.data());
    broadcast_message.set_type(msg::SSMessage::CHAT);
    if (!broadcast_message.SerializeToString(&buffer)) {
        fmt::print(stderr, "ERROR: serialization failed\nmessage was iniitalized ? {}\n", message.IsInitialized());
    }

    send_to_broadcast(buffer.data(), buffer.size());
    send_to_all_clients(message);
}

void Server::leave(Client* client) {
    connected_clients--;
    should_close = connected_clients == 0;
    clients.remove(client);
    delete client;
}

int Server::init_id() {
    int id = 255;
    struct ifaddrs* interfaces = NULL;
    struct ifaddrs* temp_addr = NULL;

    if (getifaddrs(&interfaces) == 0) {
        temp_addr = interfaces;
        while (temp_addr != NULL) {
            if (temp_addr->ifa_addr->sa_family == AF_INET && temp_addr->ifa_name != std::string_view("lo")) {
                fmt::print("Using interface {} to generate the id\n", temp_addr->ifa_name);
                id = ((struct sockaddr_in*)temp_addr->ifa_addr)->sin_addr.s_addr >> 24;
            }
            temp_addr = temp_addr->ifa_next;
        }
    }

    freeifaddrs(interfaces);
    return id;
}
