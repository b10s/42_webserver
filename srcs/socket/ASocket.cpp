#include "socket/ASocket.hpp"

#include <fcntl.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

ASocket::ASocket(lib::type::Fd fd)
    : fd_(fd), last_activity_time_(std::time(NULL)) {
  if (fd_.GetFd() == -1) {
    throw std::runtime_error("ASocket: Invalid file descriptor");
  }
  SetNonBlocking();
}

ASocket::~ASocket() {
}

int ASocket::GetFd() const {
  return fd_.GetFd();
}

void ASocket::SetNonBlocking() const {
  if (fd_.GetFd() == -1) {
    throw std::runtime_error("SetNonBlocking: Invalid file descriptor");
  }
  int flags = fcntl(fd_.GetFd(), F_GETFL, 0);
  if (flags == -1) {
    throw std::runtime_error("fcntl() failed. " + std::string(strerror(errno)));
  }
  if (fcntl(fd_.GetFd(), F_SETFL, flags | O_NONBLOCK) == -1) {
    throw std::runtime_error("fcntl() failed. " + std::string(strerror(errno)));
  }
}
