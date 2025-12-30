#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP

#include "lib/exception/ConnectionClosed.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"
#include "socket/ASocket.hpp"
#include <string>

class ClientSocket : public ASocket {
 public:
  ClientSocket(int fd, const ServerConfig& config, const std::string& client_ip);
  virtual ~ClientSocket();

  virtual SocketResult HandleEvent(int epoll_fd, uint32_t events);

 private:
  ClientSocket();
  const ServerConfig& config_;
  HttpRequest request_;
  HttpResponse response_;
  std::string output_buffer_;

  void HandleEpollIn(int epoll_fd);
  void HandleEpollOut(int epoll_fd);

  static const size_t kBufferSize = 1024;
};

#endif
