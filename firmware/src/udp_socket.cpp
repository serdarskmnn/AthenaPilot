#include "udp_socket.h"
#include <iostream>

UDPSocket::UDPSocket() {
    sockfd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) throw std::runtime_error("Failed to create UDP socket");
}

UDPSocket::~UDPSocket() {
    if (sockfd_ >= 0) {
        ::close(sockfd_);
    }
}

void UDPSocket::bind_port(unsigned short port, const std::string &addr) {
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(addr.c_str());
    int yes = 1;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if (bind(sockfd_, (sockaddr*)&saddr, sizeof(saddr)) < 0) {
        perror("bind");
        throw std::runtime_error("Failed to bind UDP socket");
    }
}

int UDPSocket::recv_from(char *buffer, size_t size, sockaddr_in &sender) {
    socklen_t sender_len = sizeof(sender);
    int r = ::recvfrom(sockfd_, buffer, size, 0, (sockaddr*)&sender, &sender_len);
    if (r < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
        perror("recvfrom");
        return -1;
    }
    return r;
}

void UDPSocket::send_to(const std::string &data, const std::string &addr, unsigned short port) {
    sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    dest.sin_addr.s_addr = inet_addr(addr.c_str());
    ssize_t r = ::sendto(sockfd_, data.c_str(), data.size(), 0, (sockaddr*)&dest, sizeof(dest));
    if (r < 0) {
        perror("sendto");
    }
}

void UDPSocket::set_nonblocking(bool nonblock) {
    int flags = fcntl(sockfd_, F_GETFL, 0);
    if (flags == -1) flags = 0;
    if (nonblock) flags |= O_NONBLOCK; else flags &= ~O_NONBLOCK;
    fcntl(sockfd_, F_SETFL, flags);
}
