#ifndef SERVER_HPP
#define SERVER_HPP

#include <array>
#include <dasynq.h>
#include <forward_list>
#include <message.pb.h>
#include <string_view>
#include <thread>
#include <unordered_map>

extern "C" {
#include "broadcast.h"
}

class Client;

/**
 * @brief Classe implémentant un serveur multi connexions
 */
class Server
{
public:
    using loop_type = dasynq::event_loop_n;

    Server(int socket, std::string_view broadcast_ip, int broadcast_port = 5000);

    ~Server() {
        close_broadcast();
    }

    int get_id() {
        return m_id;
    }

    /**
     * @brief Attends un ou plusieurs événements et exécute le code associé
     * @return false si le serveur peut s'arrêter (tous les clients sont déconnectés), true sinon
     */
    bool run();

    /**
     * @brief Envoie un message à tous les clients connectés et aux autres serveurs
     */
    void send_to_all(const msg::CSMessage& message);

    /**
     * @brief Fonction appelée par les clients pour quitter le serveur
     * @param client Un pointeur vers le client qui se déconnecte
     */
    void leave(Client* client);

private:
    int m_server_socket;
    int m_id = init_id();
    int m_nb_clients = 0;
    bool m_should_close = false;
    loop_type m_event_loop;
    std::unordered_map<std::string, int> m_client_ids;
    
    BroadcastSocket m_broadcast_sender = nullptr;
    BroadcastSocket m_broadcast_listener = nullptr;

    std::forward_list<Client*> m_clients;

    void accept_client(loop_type&, int, int);

    /**
     * @brief Envoie un message à tous les clients connectés à ce serveur
     */
    void send_to_all_clients(const msg::CSMessage& message);

    void handle_broadcast_message();

    void close_broadcast();

    /**
     * @brief Renvoie l'identifiant du serveur, pour l'instant l'id est la dernière composante
     * de l'ipv4 du serveur (xxx.xxx.xxx.id)
     */
    static int init_id();
};

#endif
