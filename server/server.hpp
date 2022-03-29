#ifndef SERVER_HPP
#define SERVER_HPP

#include <array>
#include <dasynq.h>
#include <forward_list>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <message.pb.h>

class Client;

/**
 * @brief Classe implémentant un serveur multi connexions
 */
class Server
{
public:
    using loop_type = dasynq::event_loop_n;

    /**
     * @brief Construit un serveur
     *
     * @param socket Le socket qui écoute les connexions entrantes
     */
    Server(int socket, std::string_view broadcast_ip, int broadcast_port = 5000);

    int get_id() { return m_id; }

    /**
     * @brief Attends un ou plusieurs évenements et exécute le code associé
     *
     * @return false si le serveur peut s'arrêter (tous les clients sont déconnectés), true sinon
     */
    bool run();

    /**
     * @brief Envoie un message à tous les clients connectés
     */
    void send_to_all(const msg::CSMessage& message);

    /**
     * @brief Fonction appelée par les clients pour quitter le serveur
     *
     * @param client Un pointeur vers le client qui se déconnecte
     */
    void leave(Client* client);

private:
    int server_socket;
    int m_id = init_id();
    int connected_clients = 0;
    bool should_close = false;
    loop_type event_loop;
    std::unordered_map<std::string, uint8_t> client_ids;
    uint8_t ids_sequence = 1;
    std::thread server_intercomm;

    std::forward_list<Client*> clients;

    void send_to_all_clients(const msg::CSMessage& message);

    static int init_id();
};

#endif
