#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP

#include <string>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"
#include "lib/type/Fd.hpp"
#include "socket/ASocket.hpp"

class ClientSocket : public ASocket {
 public:
  ClientSocket(lib::type::Fd fd, const ServerConfig& config,
               const std::string& client_ip);
  virtual ~ClientSocket();

  virtual SocketResult HandleEvent(int epoll_fd, uint32_t events);

 private:
  ClientSocket();
  const ServerConfig& config_;
  HttpRequest request_;
  HttpResponse response_;

  void HandleEpollIn(int epoll_fd);
  void HandleEpollOut();

  static const size_t kBufferSize = 1024;
};

#endif
