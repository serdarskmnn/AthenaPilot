#include "udp_socket.h"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>

UDPSocket::UDPSocket(){}

UDPSocket::~UDPSocket(){
    if (fd_ >= 0) close(fd_);
}

bool UDPSocket::bind_to_port(uint16_t port){
    fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_ < 0) return false;
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = INADDR_ANY;
    addr_.sin_port = htons(port);
    if (bind(fd_, (struct sockaddr *)&addr_, sizeof(addr_)) < 0){
        close(fd_);
        fd_ = -1;
        return false;
    }
    return true;
}

bool UDPSocket::send_to(const std::string &ip, uint16_t port, const std::string &data){
    if (fd_ < 0) {
        // create a socket for send
        int s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0) return false;
        struct sockaddr_in to;
        memset(&to, 0, sizeof(to));
        to.sin_family = AF_INET;
        inet_aton(ip.c_str(), &to.sin_addr);
        to.sin_port = htons(port);
        ssize_t sent = sendto(s, data.data(), data.size(), 0, (struct sockaddr*)&to, sizeof(to));
        close(s);
        return sent == (ssize_t)data.size();
    }
    struct sockaddr_in to;
    memset(&to, 0, sizeof(to));
    to.sin_family = AF_INET;
    inet_aton(ip.c_str(), &to.sin_addr);
    to.sin_port = htons(port);
    ssize_t sent = sendto(fd_, data.data(), data.size(), 0, (struct sockaddr*)&to, sizeof(to));
    return sent == (ssize_t)data.size();
}

ssize_t UDPSocket::recv_blocking(char *buf, size_t max_len, std::string &sender_ip, uint16_t &sender_port){
    if (fd_ < 0) return -1;
    struct sockaddr_in from;
    socklen_t flen = sizeof(from);
    ssize_t r = recvfrom(fd_, buf, max_len, 0, (struct sockaddr*)&from, &flen);
    if (r >= 0){
        sender_ip = inet_ntoa(from.sin_addr);
        sender_port = ntohs(from.sin_port);
    }
    return r;
}

ssize_t UDPSocket::recv_nonblocking(char *buf, size_t max_len, std::string &sender_ip, uint16_t &sender_port){
    if (fd_ < 0) return -1;
    struct sockaddr_in from;
    socklen_t flen = sizeof(from);
    ssize_t r = recvfrom(fd_, buf, max_len, MSG_DONTWAIT, (struct sockaddr*)&from, &flen);
    if (r >= 0){
        sender_ip = inet_ntoa(from.sin_addr);
        sender_port = ntohs(from.sin_port);
    }
    return r;
}

bool UDPSocket::set_blocking(bool blocking){
    if (fd_ < 0) return false;
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags < 0) return false;
    if (!blocking) flags |= O_NONBLOCK; else flags &= ~O_NONBLOCK;
    if (fcntl(fd_, F_SETFL, flags) < 0) return false;
    return true;
}
