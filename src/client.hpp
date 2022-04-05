#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "server.hpp"
#include <message.pb.h>

/**
 * @brief Classe manipulant une connexion avec un client
 */
class Client : public Server::loop_type::fd_watcher_impl<Client>
{
public:
    /**
     * @brief Crée un "gestionnaire" de connexion avec un client
     *
     * @param socket le socket à utiliser
     * @param id l'identifiant du client
     * @param server Une référence vers le serveur
     */
    Client(int socket, int id, Server& server);

    int id() {
        return m_id;
    }

    /**
     * @brief Fonction appelée lorsque le socket client contient des données prêtes à être lues
     * @param fd le socket qui est prêt (fd est égal au socket du client)
     * @return dasynq::rearm REARM pour continuer à recevoir des évenements de ce socket, REMOVE pour fermer la connection
     */
    dasynq::rearm fd_event(Server::loop_type&, int fd, int flags);

    /**
     * @brief Envoie des données vers le client
     * @return La valeur de retour de send (i.e. le nombre d'octets envoyés ou -1 en cas d'erreur)
     */
    ssize_t write(const msg::CSMessage& message);

    /**
     * @brief Fonction appelée lorsque `fd_event` renvoie REMOVE
     * La fonction quitte le serveur, cet objet client est ensuite détruit par le serveur (voir Server::leave)
     */
    void watch_removed() noexcept {
        m_server.leave(this);
    }

private:
    int m_socket;
    int m_id;
    Server& m_server;
    std::array<char, 1024> m_buffer;
    msg::CSMessage m_message;
};

#endif
