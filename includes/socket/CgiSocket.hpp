#ifndef CGISOCKET_HPP
#define CGISOCKET_HPP

#include <string>

#include "socket/ASocket.hpp"

class ClientSocket;

class CgiSocket : public ASocket {
 public:
  CgiSocket(lib::type::Fd output_fd, lib::type::Fd input_write_fd, int pid);
  virtual ~CgiSocket();

  virtual SocketResult HandleEvent(int epoll_fd, uint32_t events);
  ssize_t Send(const std::string& data);
  virtual void OnSetOwner(ClientSocket* owner);
  virtual int GetWriteFd() const;
  void CloseWriteFd();

 private:
  CgiSocket();
  int pid_;
  ClientSocket* owner_;
  lib::type::Fd input_write_fd_;
  void HandleEpollIn(int epoll_fd, SocketResult& result);
  void HandleEpollOut(int epoll_fd, SocketResult& result);

  static const size_t kBufferSize = 1024;
};

#endif
