#ifndef FD_HPP_
#define FD_HPP_

#include <unistd.h>

namespace lib {
namespace type {

class Fd {
 private:
  int fd_;

 public:
  Fd() : fd_(-1) {};
  explicit Fd(int fd) : fd_(fd) {};

  ~Fd() {
    Reset();
  };

  void Reset(int new_fd = -1) {
    if (fd_ != -1) {
      close(fd_);
    }
    fd_ = new_fd;
  }

  int GetFd() const {
    return fd_;
  }

  Fd(Fd& other) : fd_(other.fd_) {
    other.fd_ = -1;
  }

  Fd& operator=(Fd& other) {
    if (this != &other) {
      if (fd_ != -1) {
        close(fd_);
      }
      fd_ = other.fd_;
      other.fd_ = -1;
    }
    return *this;
  }
};

}  // namespace type
}  // namespace lib

#endif  // FD_HPP_
