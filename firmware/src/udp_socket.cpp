#include "udp_socket.h"
#include <stdexcept>
#include <cerrno>
#include <sstream>

UDPSocket::UDPSocket() {
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
        throw std::runtime_error("Failed to create UDP socket: " + std::to_string(errno));
    }
}

UDPSocket::~UDPSocket() {
    if (sockfd_ >= 0) close(sockfd_);
}

void UDPSocket::bindReceive(uint16_t port, const std::string& address) {
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(address.c_str());

    if (bind(sockfd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::ostringstream ss;
        ss << "UDP bind failed on " << address << ":" << port << " err=" << errno;
        throw std::runtime_error(ss.str());
    }
}

ssize_t UDPSocket::blockingReceive(char* buffer, size_t size, struct sockaddr_in* from_addr) {
    socklen_t from_len = sizeof(struct sockaddr_in);
    ssize_t ret = recvfrom(sockfd_, buffer, size, 0, reinterpret_cast<struct sockaddr*>(from_addr), &from_len);
    return ret; // blocking by design
}

ssize_t UDPSocket::sendTo(const char* buffer, size_t size, const std::string& remoteAddress, uint16_t remotePort) {
    struct sockaddr_in remote;
    std::memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_port = htons(remotePort);
    remote.sin_addr.s_addr = inet_addr(remoteAddress.c_str());

    ssize_t sent = sendto(sockfd_, buffer, size, 0, reinterpret_cast<struct sockaddr*>(&remote), sizeof(remote));
    return sent;
}