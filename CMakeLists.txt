cmake_minimum_required(VERSION 3.10)

# Proje Adı ve Versiyonu
project(AthenaPilot VERSION 0.1.0 LANGUAGES CXX)

# C++ Standardı: C++17 (Modern özellikler için ideal)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------------------------------------------------------
# KONFİGÜRASYON SEÇENEKLERİ
# --------------------------------------------------------------------------
# Bu "Switch" sayesinde tek satır değişiklikle kodu PC veya STM32 için derleyeceğiz.
option(BUILD_SITL "Build for SITL (PC Simulation)" ON)

# Derleme Uyarıları (Kod kalitesi için sıkı tutuyoruz)
if(MSVC)
    add_compile_options(/W4) # Windows Visual Studio için
else()
    add_compile_options(-Wall -Wextra) # GCC/Clang için
endif()

# --------------------------------------------------------------------------
# KAYNAK DOSYALARIN TOPLANMASI
# --------------------------------------------------------------------------
# src klasörü altındaki tüm .cpp dosyalarını bulur
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Header dosyalarımızın olduğu klasörü tanıtıyoruz
include_directories(include)

# İleride MAVLink kütüphanesini buraya koyacağız
include_directories(src/lib) 
include_directories(src/lib/mavlink)

# --------------------------------------------------------------------------
# PLATFORM AYARLARI (SITL vs HARDWARE)
# --------------------------------------------------------------------------
if(BUILD_SITL)
    message(STATUS "🟢 Build Modu: SITL (PC Simülasyonu)")
    # Derleme Uyarıları
if(MSVC)
    add_compile_options(/W4)
else()
    # -Wall -Wextra açıyoruz ama MAVLink'in packed-member uyarısını susturuyoruz
    add_compile_options(-Wall -Wextra -Wno-address-of-packed-member)
endif()
    
    # PC Uygulamasını oluştur (Executable)
    add_executable(AthenaPilot ${SOURCES})

    # Windows kullanıyorsan Socket kütüphanesini bağla (UDP için gerekli)
    if(WIN32)
        target_link_libraries(AthenaPilot ws2_32)
    endif()

else()
    message(STATUS "🟠 Build Modu: STM32 Hardware (Henüz aktif değil)")
    # İleride buraya arm-none-eabi-gcc ayarları ve linker script gelecek.
endif()