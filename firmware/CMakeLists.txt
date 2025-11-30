cmake_minimum_required(VERSION 3.16)
project(AthenaPilotFirmware LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(athena_firmware
    src/main.cpp
    src/pid.cpp
    src/mixer.cpp
    src/udp_socket.cpp
)

# Link libraries if needed (pthread for select/timers if used)
find_package(Threads REQUIRED)
target_link_libraries(athena_firmware PRIVATE Threads::Threads)

# Optionally install
install(TARGETS athena_firmware DESTINATION bin)
