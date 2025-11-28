#ifndef UDP_DRIVER_H
#define UDP_DRIVER_H

#include <string>
#include <cstdint>

// Linux Socket kütüphaneleri
#include <netinet/in.h>

class UdpDriver {
public:
    UdpDriver();
    ~UdpDriver();

    // Sürücüyü başlatır ve belirtilen portu dinlemeye başlar
    bool init(const std::string& target_ip, int target_port, int local_port);

    // Veri gönderir (Non-blocking)
    int write(const uint8_t* buffer, int length);

    // Veri okur (Non-blocking)
    // Dönüş: Okunan bayt sayısı. 0 ise veri yok.
    int read(uint8_t* buffer, int max_length);

private:
    int socket_fd;               // Socket dosya tanımlayıcısı
    struct sockaddr_in remote_addr; // Hedef adres (Simulink/MATLAB)
    struct sockaddr_in local_addr;  // Kendi adresimiz
    bool is_initialized;
};

#endif // UDP_DRIVER_H