/*
 * inspiré de https://github.com/atwilc3000/sample/blob/master/Bluetooth/rfcomm_server.c
 */
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <cstdio>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <fmt/core.h>
#include <optional>
#include <string>

class Client
{
public:
    Client(std::string_view name, int socket) : name(name), socket(socket) {}
    Client(std::string&& name, int socket) : name(name), socket(socket) {}

    ~Client() {
        if (socket != -1) {
            close(socket);
        }
    }

    std::optional<std::string> read() {
        auto bytes_read = recv(socket, buffer.data(), buffer.size(), 0);
        if (bytes_read == -1) {
            return std::nullopt;
        } else {
            return std::string(buffer.data(), bytes_read);
        }
    }

    ssize_t write(std::string_view data) {
        ssize_t bytes_written = send(socket, data.data(), data.size(), 0);
        return bytes_written;
    }

    const std::string& getName() const {
        return name;
    }

private:
    std::array<char, 1024> buffer;
    std::string name;
    int socket = -1;
};

Client accept_client(int server_socket) {
    sockaddr_rc client_address = {0};
    unsigned int addr_len;

    int socket = accept(server_socket, (sockaddr*)&client_address, &addr_len);

    std::string buffer(18, '\0');
    ba2str(&client_address.rc_bdaddr, buffer.data());
    fmt::print("Connected from {}\n", buffer);

    return {buffer, socket};
}

size_t read(int socket, std::string& buffer) {
    buffer.resize(1024);
    size_t bytes_read = recv(socket, buffer.data(), buffer.size(), 0);
    if (bytes_read != -1) {
        buffer.resize(bytes_read);
    }
    return bytes_read;
}

int main(int argc, char** argv) {
    const bdaddr_t bdaddr_any = {{0, 0, 0, 0, 0, 0}};
    sockaddr_rc local_address = {0};

    std::string buffer(1024, '\0');

    fmt::print("Start Bluetooth RFCOMM server...\n");

    /* allocate socket */
    int server_sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    local_address.rc_family = AF_BLUETOOTH;
    bacpy(&local_address.rc_bdaddr, &bdaddr_any);
    local_address.rc_channel = 1;

    fmt::print("binding\n");
    if (bind(server_sock, (sockaddr*)&local_address, sizeof(local_address)) < 0) {
        perror("failed to bind");
        exit(1);
    }

    fmt::print("listening\n");
    listen(server_sock, 1);

    Client client = accept_client(server_sock);

    /* read data from the client */
    size_t bytes_sent;
    while (auto msg = client.read()) {
        std::string message = msg.value();
        fmt::print("{} sent '{}'\n", client.getName(), message);
        for (char& c : message) {
            c = std::toupper(c);
        }
        bytes_sent = client.write(message);
        // bytes_sent = send(client_sock, buffer.data(), buffer.size(), 0);
        fmt::print("Sent {} bytes to client: '{}'\n", bytes_sent, message);
    }

    /* close connection */
    close(server_sock);
    return 0;
}
