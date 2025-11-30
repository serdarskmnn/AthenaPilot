#pragma once

#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>

class UDPSocket {
public:
    UDPSocket();
    ~UDPSocket();

    void bind_port(unsigned short port, const std::string &addr = "127.0.0.1");
    int recv_from(char *buffer, size_t size, sockaddr_in &sender);
    void send_to(const std::string &data, const std::string &addr, unsigned short port);
    void set_nonblocking(bool nonblock);

private:
    int sockfd_;
};
