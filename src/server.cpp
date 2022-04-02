#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <ifaddrs.h>
#include <unistd.h>

#include "client.hpp"
#include "server.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <string>

using rearm = dasynq::rearm;

void print_error(std::string_view error_message) {
    static const std::string prefix = fmt::format(fmt::emphasis::bold | fmt::fg(fmt::color::red), "ERREUR: ");
    fmt::print("{}: {}\n", prefix, error_message);
}

Server::Server(int socket, std::string_view broadcast_ip, int broadcast_port) : m_server_socket(socket) {
    fmt::print("Id du serveur : {}\n", get_id());

    m_broadcast_listener = broadcastSocket(nullptr, broadcast_port, BROADCAST_RECEIVE);
    m_broadcast_sender = broadcastSocket(broadcast_ip.data(), broadcast_port, BROADCAST_SEND);

    if (m_broadcast_listener == nullptr || m_broadcast_sender == nullptr) {
        print_error("Echec lors de la création des sockets de broadcast");
        m_should_close = true;
        return;
    }

    // gere les messages de broadcast
    loop_type::fd_watcher::add_watch(m_event_loop, m_broadcast_listener->socket, dasynq::IN_EVENTS, [&](loop_type&, int, int) {
        handle_broadcast_message();
        return rearm::REARM;
    });

    // gere les inputs sur l'entree standard
    loop_type::fd_watcher::add_watch(m_event_loop, fileno(stdin), dasynq::IN_EVENTS, [&](loop_type&, int, int) {
        static std::string input;
        std::getline(std::cin, input);
        if (input == "stop") {
            msg::CSMessage stop_message;
            stop_message.set_clientid(0);
            stop_message.set_serverid(m_id);
            stop_message.set_data("stop");
            send_to_all_clients(stop_message);
            m_should_close = true;
            return rearm::REMOVE;
        } else if (input == "stat") {
            fmt::print("{} clients sont connectes\n", m_nb_clients);
        }
        return rearm::REARM;
    });

    // gere les nouvelles connexions
    loop_type::fd_watcher::add_watch(m_event_loop, m_server_socket, dasynq::IN_EVENTS, [&](loop_type& loop, int sock, int flags) {
        accept_client(loop, sock, flags);
        return rearm::REARM;
    });
}

bool Server::run() {
    if (m_should_close) {
        close_broadcast();
        return false;
    } else {
        m_event_loop.run();
        return true;
    }
}

void Server::send_to_all_clients(const msg::CSMessage& message) {
    for (auto it = m_clients.begin(); it != m_clients.end();) {
        if ((*it)->id() != message.clientid() && (*it)->write(message) == -1) {
            auto current = it++;
            (*current)->deregister(m_event_loop);
        } else {
            ++it;
        }
    }
}

void Server::send_to_all(const msg::CSMessage& message) {
    static msg::SSMessage broadcast_message; // TODO : déplacer dans les variables membres
    static std::string buffer;
    broadcast_message.set_serverid(m_id);
    broadcast_message.set_clientid(message.clientid());
    broadcast_message.set_data(message.data());
    broadcast_message.set_type(msg::SSMessage::CHAT);
    if (!broadcast_message.SerializeToString(&buffer)) {
        print_error("Echec de la serialisation d'un message");
    } else {
        sendBroadcastMessage(m_broadcast_sender, buffer.data(), buffer.size());
        send_to_all_clients(message);
    }
}

void Server::leave(Client* client) {
    m_nb_clients--;
    m_clients.remove(client);
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
                fmt::print("Utilisation de l'interface {} pour la generation de l'id\n", temp_addr->ifa_name);
                id = ((struct sockaddr_in*)temp_addr->ifa_addr)->sin_addr.s_addr >> 24;
            }
            temp_addr = temp_addr->ifa_next;
        }
    }

    freeifaddrs(interfaces);
    return id;
}

void Server::accept_client(loop_type& event_loop, int sock, int flags) {
    static int next_id = m_id << 16;
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

        auto it = m_client_ids.find(address);
        int client_id;
        if (it == m_client_ids.end()) {
            client_id = next_id++;
            m_client_ids[address] = client_id;
        } else {
            client_id = it->second;
        }

        Client* client = new Client(client_socket, client_id, *this);
        m_nb_clients++;
        client->add_watch(event_loop, client_socket, flags);
        m_clients.push_front(client);
    }
}

void Server::handle_broadcast_message() {
    constexpr size_t buffer_size = 1024;
    static std::array<char, buffer_size> buffer;
    static msg::CSMessage client_message;
    static msg::SSMessage message;

    int size = receiveBroadcastMessage(m_broadcast_listener, buffer.data(), buffer.size());
    if (size > 0) {
        fmt::print("{} octets recus\n", size);
        if (!message.ParseFromArray(buffer.data(), size)) {
            print_error("Echec de la deserialisation d'un message de broadcast");
            return;
        }

        if (message.serverid() != m_id && message.type() == msg::SSMessage::CHAT) {
            fmt::print("Recu de {}:{} : '{}'\n", message.serverid(), message.clientid(), message.data());
            client_message.set_clientid(message.clientid());
            client_message.set_data(message.data());
            send_to_all_clients(client_message);
        }
    } else if (size == 0) {
        print_error("Deconnexion du socket de diffusion");
        return;
    } else if (size == -1) {
        print_error("Echec de la lecture du socket de diffusion");
        return;
    }
}

void Server::close_broadcast() {
    if (m_broadcast_listener != nullptr) {
        closeBroadcastSocket(m_broadcast_listener);
        m_broadcast_listener = nullptr;
    }
    if (m_broadcast_sender != nullptr) {
        closeBroadcastSocket(m_broadcast_sender);
        m_broadcast_sender = nullptr;
    }
}
