#include "ServerSocket.hpp"
#include <iostream>

ServerSocket::ServerSocket(int fd) : ASocket(fd) {}

ServerSocket::~ServerSocket() {}

void ServerSocket::HandleEvent(uint32_t events) {
    (void)events;
    std::cout << "ServerSocket HandleEvent" << std::endl;
}
