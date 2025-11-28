#include "udp_driver.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

UdpDriver::UdpDriver() : socket_fd(-1), is_initialized(false) {
}

UdpDriver::~UdpDriver() {
    if (socket_fd >= 0) {
        close(socket_fd);
    }
}

bool UdpDriver::init(const std::string& target_ip, int target_port, int local_port) {
    // 1. Socket oluştur (IPv4, UDP)
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        std::cerr << "UDP Driver: Socket olusturulamadi!" << std::endl;
        return false;
    }

    // 2. Socket'i Non-Blocking moda al
    // (Veri yoksa program donmasın, hemen devam etsin)
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);

    // 3. Kendi adresimizi ayarla (Bind)
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY; // Tüm arayüzlerden dinle
    local_addr.sin_port = htons(local_port);

    if (bind(socket_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        std::cerr << "UDP Driver: Port baglanamadi (Bind Failed): " << local_port << std::endl;
        close(socket_fd);
        return false;
    }

    // 4. Hedef adresi ayarla (MATLAB/Simulink burası olacak)
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(target_port);
    if (inet_pton(AF_INET, target_ip.c_str(), &remote_addr.sin_addr) <= 0) {
        std::cerr << "UDP Driver: Gecersiz IP Adresi" << std::endl;
        return false;
    }

    is_initialized = true;
    std::cout << "UDP Driver Baslatildi. Local: " << local_port << " -> Hedef: " << target_ip << ":" << target_port << std::endl;
    return true;
}

int UdpDriver::write(const uint8_t* buffer, int length) {
    if (!is_initialized) return -1;
    
    // sendto fonksiyonu datayı doğrudan hedef adrese fırlatır
    return sendto(socket_fd, buffer, length, 0, 
                  (struct sockaddr*)&remote_addr, sizeof(remote_addr));
}

int UdpDriver::read(uint8_t* buffer, int max_length) {
    if (!is_initialized) return -1;

    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    // recvfrom veriyi alır. Non-blocking olduğu için veri yoksa -1 döner (Hata değil, durumdur)
    int len = recvfrom(socket_fd, buffer, max_length, 0, 
                       (struct sockaddr*)&sender_addr, &sender_len);
    
    if (len > 0) {
        // Veri geldi!
        return len;
    }
    return 0; // Veri yok
}