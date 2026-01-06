#ifndef CGISOCKET_HPP
#define CGISOCKET_HPP

#include <string>

#include "socket/ASocket.hpp"

class ClientSocket;

class CgiSocket : public ASocket {
 public:
  CgiSocket(lib::type::Fd fd, int pid);
  virtual ~CgiSocket();

  virtual SocketResult HandleEvent(int epoll_fd, uint32_t events);
  ssize_t Send(const std::string& data);
  std::string Receive();
  virtual void OnSetOwner(ClientSocket* owner);

 private:
  CgiSocket();
  int pid_;
  ClientSocket* owner_;

  static const size_t kBufferSize = 1024;
};

#endif
