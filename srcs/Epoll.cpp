#include "Epoll.hpp"
#include "HttpResponse.hpp"
#include "lib/utils/Bzero.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <iostream>

Epoll::Epoll() {
  lib::utils::Bzero(&server_addr_, sizeof(server_addr_));
}

Epoll::~Epoll() {
  close(server_fd_);
  close(epoll_fd_);
}

void Epoll::CreateSocket() {
  server_fd_ = socket(PF_INET, SOCK_STREAM, 0);
  if (server_fd_ == -1) {
    // throw error;
  }
}

void Epoll::SetServerAddr() {
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_addr.s_addr = INADDR_ANY;
  server_addr_.sin_port = htons(kPort);
}

void Epoll::BindSocket() {
  int ret;
  ret = bind(server_fd_, (sockaddr *)&server_addr_, sizeof(sockaddr_in));
  if (ret == -1) {
    // throw error;
  }
}

void Epoll::ListenSocket() {
  int ret;
  ret = listen(server_fd_, SOMAXCONN);
  if (ret == -1) {
    // throw error;
  }
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

int Epoll::GetServerFd() {
  return server_fd_;
}

sockaddr_in *Epoll::GetServerAddr() {
  return &server_addr_;
}

int Epoll::GetEpollFd() {
  return epoll_fd_;
}

