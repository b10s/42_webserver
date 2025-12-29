#ifndef SERVERSOCKET_HPP
#define SERVERSOCKET_HPP

#include "ASocket.hpp"

class ServerSocket : public ASocket {
 public:
  ServerSocket(int fd);
  virtual ~ServerSocket();

  virtual void HandleEvent(uint32_t events);
};

#endif
