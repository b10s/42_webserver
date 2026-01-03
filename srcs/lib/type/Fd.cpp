#include "lib/type/Fd.hpp"

#include <unistd.h>

namespace lib {
namespace type {

Fd::Fd() : fd_(-1) {
}

Fd::Fd(int fd) : fd_(fd) {
}

Fd::~Fd() {
  Reset();
}

void Fd::Reset(int new_fd) {
  if (fd_ != -1) {
    close(fd_);
  }
  fd_ = new_fd;
}

int Fd::GetFd() const {
  return fd_;
}

Fd::Fd(const Fd& other) : fd_(other.fd_) {
  const_cast<Fd&>(other).fd_ = -1;
}

Fd& Fd::operator=(const Fd& other) {
  if (this != &other) {
    if (fd_ != -1) {
      close(fd_);
    }
    fd_ = other.fd_;
    const_cast<Fd&>(other).fd_ = -1;
  }
  return *this;
}

}  // namespace type
}  // namespace lib
