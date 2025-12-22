#include "Epoll.hpp"

#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "HttpResponse.hpp"
#include "lib/utils/Bzero.hpp"

Epoll::Epoll() {
}

Epoll::~Epoll() {
  for (std::vector<int>::iterator it = server_fds_.begin(); it != server_fds_.end(); ++it) {
    close(*it);
  }
  close(epoll_fd_);
}

void Epoll::AddServer(unsigned short port) {
  int server_fd = socket(PF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    // throw error;
  }

  sockaddr_in server_addr;
  lib::utils::Bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  int ret = bind(server_fd, (sockaddr *)&server_addr, sizeof(sockaddr_in));
  if (ret == -1) {
    // throw error;
  }

  ret = listen(server_fd, SOMAXCONN);
  if (ret == -1) {
    // throw error;
  }

  AddSocketToInstance(server_fd);
  server_fds_.push_back(server_fd);
}

void Epoll::CreateInstance() {
  epoll_fd_ = epoll_create1(0);
  if (epoll_fd_ == -1) {
    // throw error;
  }
}

void Epoll::AddSocketToInstance(int socket_fd) {
  int ret;
  epoll_event ev;

  ev.events = EPOLLIN;
  ev.data.fd = socket_fd;
  ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket_fd, &ev);
  if (ret == -1) {
    // throw error;
  }
}

int Epoll::Wait() {
  return epoll_wait(epoll_fd_, events_, kMaxEvents, -1);
}

bool Epoll::IsServerFd(int fd) {
  for (std::vector<int>::iterator it = server_fds_.begin(); it != server_fds_.end(); ++it) {
    if (*it == fd) {
      return true;
    }
  }
  return false;
}

epoll_event* Epoll::GetEvents() {
  return events_;
}
