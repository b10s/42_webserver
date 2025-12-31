#ifndef FD_HPP_
#define FD_HPP_

#include <unistd.h>

namespace lib {
namespace type {

class Fd {
 private:
  int fd_;

 public:
  explicit Fd(int fd) : fd_(fd) {};

  ~Fd() {
    if (fd_ != -1) {
      close(fd_);
      fd_ = -1;
    }
  };

  operator int() const {
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

 private:
  Fd();
};

}  // namespace type
}  // namespace lib

#endif  // FD_HPP_
