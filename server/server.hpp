#ifndef SERVER_HPP
#define SERVER_HPP

#include <array>
#include <dasynq.h>
#include <forward_list>
#include <string_view>

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
    Server(int socket);

    /**
     * @brief Attends un ou plusieurs évenements et exécute le code associé
     *
     * @return false si le serveur peut s'arrêter (tous les clients sont déconnectés), true sinon
     */
    bool run();

    /**
     * @brief Envoie un message à tous les clients connectés
     */
    void send_to_all(std::string_view message);

    /**
     * @brief Fonction appelée par les clients pour quitter le serveur
     *
     * @param client Un pointeur vers le client qui se déconnecte
     */
    void leave(Client* client);

private:
    int server_socket;
    int connected_clients = 0;
    bool should_close = false;
    loop_type event_loop;
    std::forward_list<Client*> clients;
};

/**
 * @brief Classe manipulant une connexion avec un client
 */
class Client : public Server::loop_type::fd_watcher_impl<Client>
{
public:
    /**
     * @brief Crée un nouveau client
     *
     * @param socket le socket attribué à cette connexion
     * @param chat_room le serveur auquel le client est connecté
     */
    Client(int socket, Server& chat_room);

    /**
     * @brief Fonction appelée lorsque le socket client contient des données prêtes à être lues
     *
     * @param fd le socket qui est prêt (fd est égal au socket du client)
     * @param flags les flags passés lors de l'ajout du client à la boucle d'évenements
     * @return dasynq::rearm REARM pour continuer à recevoir des évenements de ce socket, REMOVE pour fermer la connection
     */
    dasynq::rearm fd_event(Server::loop_type&, int fd, int flags);

    /**
     * @brief Envoie des données vers le client
     *
     * @param message Les données à envoyer
     * @return ssize_t La valeur de retour de send (i.e. le nombre d'octets envoyés ou -1 en cas d'erreur)
     */
    ssize_t write(std::string_view message);

    /**
     * @brief Fonction appelée lorsque `fd_event` renvoie REMOVE
     *
     * La fonction quitte le serveur, cet objet client est ensuite détruit par le serveur (voir Server::leave)
     */
    void watch_removed() noexcept {
        server.leave(this);
    }

private:
    int socket;
    Server& server;
    std::array<char, 1024> buffer;
};

#endif
