#ifndef SERVERSOCKET_HPP
#define SERVERSOCKET_HPP

#include "ServerConfig.hpp"
#include "socket/ASocket.hpp"

class ServerSocket : public ASocket {
 public:
  explicit ServerSocket(const ServerConfig& config);
  virtual ~ServerSocket();

  virtual SocketResult HandleEvent(int epoll_fd, uint32_t events);

 private:
  ServerSocket();
  const ServerConfig& config_;
};

#endif
