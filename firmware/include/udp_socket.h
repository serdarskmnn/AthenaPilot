// Simple UDP wrapper for Linux sockets
#ifndef ATHENAPILOT_UDP_SOCKET_H
#define ATHENAPILOT_UDP_SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <stdexcept>
#include <cstring>
#include <iostream>

class UDPSocket {
public:
    UDPSocket();
    ~UDPSocket();

    // Bind the socket for receiving on the given local port
    void bindReceive(uint16_t port, const std::string& address = "127.0.0.1");

    // Blocking receive (wraps recvfrom). Returns the number of bytes received.
    ssize_t blockingReceive(char* buffer, size_t size, struct sockaddr_in* from_addr);

    // Send bytes to a remote address/port
    ssize_t sendTo(const char* buffer, size_t size, const std::string& remoteAddress, uint16_t remotePort);

private:
    int sockfd_; 
};

#endif // ATHENAPILOT_UDP_SOCKET_H