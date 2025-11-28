% AthenaPilot Java Tabanlı Bağlantı Testi
clear; clc;

localPort = 14550; % Dinlediğimiz kapı
fprintf('AthenaPilot Dinleniyor (Port: %d)...\n', localPort);

import java.io.*
import java.net.*

try
    % Java DatagramSocket oluştur
    socket = DatagramSocket(localPort);
    socket.setSoTimeout(5000); % 5 saniye bekle, gelmezse kapat
    
    % Boş bir paket hazırla (Buffer)
    buffer = zeros(1, 1024, 'int8');
    packet = DatagramPacket(buffer, length(buffer));
    
    % Veriyi bekle
    socket.receive(packet);
    
    % Veri geldiyse buraya düşer
    fprintf('[BAŞARILI] Veri yakalandı! Uzunluk: %d byte.\n', packet.getLength());
    fprintf('Firmware ile bağlantı sağlam.\n');
    
    socket.close();
    
catch ME
    % Hata veya zaman aşımı
    if exist('socket','var')
        socket.close();
    end
    if contains(ME.message, 'Receive timed out')
        fprintf('[HATA] 5 saniye içinde veri gelmedi. Firmware çalışıyor mu?\n');
    else
        fprintf('[HATA] %s\n', ME.message);
    end
end