#ifndef SERVER_HPP
#define SERVER_HPP

#include <dasynq.h>
#include <forward_list>
#include <string_view>
#include <array>

class ClientWatcher;

class ChatRoom
{
public:
    using loop_type = dasynq::event_loop_n;

    ChatRoom(int socket, loop_type& event_loop);

    bool should_stop() const { return should_close; }

    void send(std::string_view message);

    void leave(ClientWatcher* client);

private:
    int server_socket;
    int connected_clients = 0;
    bool should_close = false;
    loop_type& event_loop;
    std::forward_list<ClientWatcher*> clients;
};

class ClientWatcher : public ChatRoom::loop_type::fd_watcher_impl<ClientWatcher>
{
public:
    ClientWatcher(int socket, ChatRoom& chat_room) ;

    dasynq::rearm fd_event(ChatRoom::loop_type&, int fd, int flags);

    ssize_t write(std::string_view message);

    void watch_removed() noexcept {
        chat.leave(this);
    }

private:
    int socket;
    ChatRoom& chat;
    std::array<char, 1024> buffer;
};

#endif
