#include "ClientSocket.hpp"
#include <iostream>

ClientSocket::ClientSocket(int fd) : ASocket(fd) {}

ClientSocket::~ClientSocket() {}

void ClientSocket::HandleEvent(uint32_t events) {
    (void)events;
    std::cout << "ClientSocket HandleEvent" << std::endl;
}
