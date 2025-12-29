#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP

#include "ASocket.hpp"

class ClientSocket : public ASocket {
 public:
  ClientSocket(int fd);
  virtual ~ClientSocket();

  virtual void HandleEvent(uint32_t events);
};

#endif
