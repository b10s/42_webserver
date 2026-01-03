#include "socket/ASocket.hpp"

#include <fcntl.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

ASocket::ASocket(lib::type::Fd fd) : fd_(fd) {
}

ASocket::ASocket() {
}

ASocket::~ASocket() {
}

int ASocket::GetFd() const {
  return fd_.GetFd();
}

void ASocket::SetNonBlocking() const {
  int flags = fcntl(fd_.GetFd(), F_GETFL, 0);
  if (flags == -1) {
    throw std::runtime_error("fcntl() failed. " + std::string(strerror(errno)));
  }
  if (fcntl(fd_.GetFd(), F_SETFL, flags | O_NONBLOCK) == -1) {
    throw std::runtime_error("fcntl() failed. " + std::string(strerror(errno)));
  }
}
