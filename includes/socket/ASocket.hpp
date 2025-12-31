#ifndef ASOCKET_HPP
#define ASOCKET_HPP

#include <stdint.h>
#include <unistd.h>

class ASocket;

struct SocketResult {
  ASocket* new_socket;
  bool remove_socket;

  SocketResult() : new_socket(NULL), remove_socket(false) {
  }
};

class ASocket {
 public:
  explicit ASocket(int fd);
  virtual ~ASocket();

  virtual SocketResult HandleEvent(int epoll_fd, uint32_t events) = 0;
  int GetFd() const;

 protected:
  int fd_;

 private:
  ASocket();
};

#endif
