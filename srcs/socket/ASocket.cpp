#include "socket/ASocket.hpp"

ASocket::ASocket(int fd) : fd_(fd) {
}

ASocket::~ASocket() {
  if (fd_ != -1) {
    close(fd_);
    fd_ = -1;
  }
}

int ASocket::GetFd() const {
  return fd_;
}
