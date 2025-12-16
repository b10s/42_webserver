#ifndef EPOLL_HPP_
#define EPOLL_HPP_

#include <netinet/in.h>
#include <sys/epoll.h>

class Epoll {
public:
  Epoll();
  ~Epoll();
  void CreateSocket();
  void SetServerAddr(unsigned short port);
  void BindSocket();
  void ListenSocket();
  void CreateInstance();
  void AddSocketToInstance(int socket_fd);
  int GetServerFd();
  int GetEpollFd();
  sockaddr_in *GetServerAddr();

  static const int kMaxEvents = 10;

private:
  int server_fd_;
  int epoll_fd_;
  sockaddr_in server_addr_;
};

#endif
