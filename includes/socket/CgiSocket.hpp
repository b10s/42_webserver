#ifndef CGISOCKET_HPP
#define CGISOCKET_HPP

#include <string>

#include "socket/ASocket.hpp"

class CgiSocket : public ASocket {
 public:
  explicit CgiSocket(lib::type::Fd fd);
  virtual ~CgiSocket();

  virtual SocketResult HandleEvent(int epoll_fd, uint32_t events);

  ssize_t Send(const std::string& data);
  std::string Receive();

 private:
  CgiSocket();
};

#endif
