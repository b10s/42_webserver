#ifndef EPOLL_HPP_
#define EPOLL_HPP_

#include <netinet/in.h>
#include <sys/epoll.h>

#include <vector>

class Epoll {
 public:
  Epoll();
  ~Epoll();
  void CreateInstance();
  void AddServer(unsigned short port);
  void AddSocketToInstance(int socket_fd);
  int Wait();
  bool IsServerFd(int fd);
  epoll_event* GetEvents();

  static const int kMaxEvents = 10;

 private:
  std::vector<int> server_fds_;
  int epoll_fd_;
  epoll_event events_[kMaxEvents];
};

#endif
