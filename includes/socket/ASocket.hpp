#ifndef ASOCKET_HPP
#define ASOCKET_HPP

#include <stdint.h>
#include <unistd.h>

#include <ctime>
#include <string>

#include "lib/type/Fd.hpp"

class ASocket;
class ClientSocket;

struct SocketResult {
  ASocket* new_socket;
  bool remove_socket;

  SocketResult() : new_socket(NULL), remove_socket(false) {
  }
};

class ASocket {
 public:
  explicit ASocket(lib::type::Fd fd);
  virtual ~ASocket();

  virtual void UpdateLastActivity() {
    last_activity_time_ = std::time(NULL);
  }

  virtual bool IsTimeout(time_t threshold_time) const {
    return last_activity_time_ < threshold_time;
  }

  virtual SocketResult HandleEvent(int epoll_fd, uint32_t events) = 0;

  virtual void OnSetOwner(ClientSocket* owner) {
    (void)owner;
  }

  int GetFd() const;

 protected:
  lib::type::Fd fd_;
  time_t last_activity_time_;
  std::string read_buffer_;
  std::string write_buffer_;
  void SetNonBlocking() const;
};

#endif
