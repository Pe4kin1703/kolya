cmake_minimum_required(VERSION 3.14)
project(server)

set(CMAKE_CXX_STANDARD 20)

set(S src/sources/)
set(H src/headers/)

set(SOURCES ${S}server.cpp)
set(HEADERS ${H}server.h)

add_executable(server ${S}main.cpp ${HEADERS} ${SOURCES})

target_include_directories(server PUBLIC ${H})

target_link_libraries(server pthread)

