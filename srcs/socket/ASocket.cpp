#include "socket/ASocket.hpp"

ASocket::ASocket(lib::type::Fd fd) : fd_(fd) {
}

ASocket::ASocket() {
}

ASocket::~ASocket() {
}

int ASocket::GetFd() const {
  return fd_.GetFd();
}
