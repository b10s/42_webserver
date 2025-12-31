#include "socket/ASocket.hpp"

ASocket::ASocket(int fd) : fd_(fd) {
}

ASocket::~ASocket() {
}

int ASocket::GetFd() const {
  return fd_;
}
