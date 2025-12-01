#pragma once

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class UDPSocket {
public:
    UDPSocket();
    ~UDPSocket();

    // Bind to a local port (for listening)
    bool bind_to_port(uint16_t port);

    // Send to IP/port
    bool send_to(const std::string &ip, uint16_t port, const std::string &data);

    // Blocking receive (for Simulink - lock-step)
    ssize_t recv_blocking(char *buf, size_t max_len, std::string &sender_ip, uint16_t &sender_port);

    // Non-blocking receive (for Python RC listener)
    ssize_t recv_nonblocking(char *buf, size_t max_len, std::string &sender_ip, uint16_t &sender_port);

    // Set socket blocking or non-blocking
    bool set_blocking(bool blocking);

private:
    int fd_ = -1;
    struct sockaddr_in addr_;
};
