cmake_minimum_required(VERSION 3.10)
project(AthenaPilotFirmware VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(athena_firmware ${SOURCES})

target_include_directories(athena_firmware PRIVATE ${CMAKE_SOURCE_DIR}/include)
