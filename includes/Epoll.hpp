#ifndef EPOLL_HPP_
#define EPOLL_HPP_

#include <netinet/in.h>
#include <sys/epoll.h>

class Epoll {
public:
  Epoll();
  ~Epoll();
  void CreateSocket();
  void SetServerAddr();
  void BindSocket();
  void ListenSocket();
  void CreateInstance();
  void AddSocketToInstance(int socket_fd);

  int GetServerFd();
  int GetEpollFd();
  sockaddr_in *GetServerAddr();

  void Loop();

private:
  static const int kPort = 8000;
  static const int kMaxEvents = 10;
  int server_fd_;
  sockaddr_in server_addr_;
  int epoll_fd_;
  epoll_event events_[kMaxEvents];
};

#endif
