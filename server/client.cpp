#include "client.hpp"
#include <fmt/core.h>
#include <sys/socket.h>

using dasynq::rearm;

Client::Client(int socket, int id, Server& server) : m_socket(socket), m_id(id), m_server(server) {
    m_message.set_clientid(m_id);
    m_message.set_serverid(m_server.get_id());
}

rearm Client::fd_event(Server::loop_type&, int fd, int flags) {
    ssize_t byte_count = recv(fd, m_buffer.data(), m_buffer.size(), 0);

    if (byte_count == -1) {
        return rearm::REMOVE;
    } else {
        m_message.set_data(m_buffer.data(), byte_count);
        fmt::print("Recu de {}:{} : '{}'\n", m_message.serverid(), m_message.clientid(), m_message.data());
        if (m_message.data() == "stop") {
            write(m_message);
            close(fd);
            return rearm::REMOVE;
        } else {
            m_server.send_to_all(m_message);
        }
        return rearm::REARM;
    }
}

ssize_t Client::write(const msg::CSMessage& message) {
    return send(m_socket, message.data().data(), message.data().size(), 0);
}
