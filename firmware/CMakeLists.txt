cmake_minimum_required(VERSION 3.10)
project(AthenaPilotFirmware LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json to help editor tools like cpptools/clangd find include paths
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(athena_firmware
    src/main.cpp
    src/pid.cpp
    src/mixer.cpp
    src/udp_socket.cpp
)

target_include_directories(athena_firmware PRIVATE include)
target_compile_options(athena_firmware PRIVATE -Wall -Wextra -Wpedantic)
