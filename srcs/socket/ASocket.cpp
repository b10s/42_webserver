#include "socket/ASocket.hpp"

ASocket::ASocket(lib::type::Fd& fd) : fd_(fd) {
}

ASocket::~ASocket() {
}

lib::type::Fd& ASocket::GetFd() const {
  return fd_;
}
