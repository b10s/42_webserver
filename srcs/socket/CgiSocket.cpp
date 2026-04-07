#include "socket/CgiSocket.hpp"

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

#include "lib/exception/ResponseStatusException.hpp"
#include "lib/type/Fd.hpp"
#include "lib/utils/file_utils.hpp"
#include "socket/ClientSocket.hpp"

CgiSocket::CgiSocket(lib::type::Fd output_fd, lib::type::Fd input_write_fd,
                     int pid)
    : ASocket(output_fd),
      pid_(pid),
      owner_(NULL),
      input_write_fd_(input_write_fd) {
  int flags = fcntl(input_write_fd_.GetFd(), F_GETFL, 0);
  if (flags != -1) {
    fcntl(input_write_fd_.GetFd(), F_SETFL, flags | O_NONBLOCK);
  }
}

CgiSocket::~CgiSocket() {
  if (owner_) {
    owner_->RemoveCgiSocket(this);
  }
  if (pid_ > 0) {
    kill(pid_, SIGTERM);
    waitpid(pid_, NULL, 0);
  }
}

SocketResult CgiSocket::HandleEvent(int epoll_fd, uint32_t events) {
  SocketResult result;
  try {
    if (events & EPOLLERR) {
      if (owner_) {
        owner_->OnCgiExecutionError(epoll_fd);
      }
      result.remove_socket = true;
    } else {
      if (events & (EPOLLIN | EPOLLHUP)) {
        HandleEpollIn(epoll_fd, result);
      }
      if (events & EPOLLOUT) {
        HandleEpollOut(epoll_fd, result);
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "CgiSocket error: " << e.what() << std::endl;
    result.remove_socket = true;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd_.GetFd(), NULL) == -1) {
      std::cerr << "epoll_ctl error" << std::endl;
    }
  }
  return result;
}

void CgiSocket::HandleEpollIn(int epoll_fd, SocketResult& result) {
  char buf[kBufferSize];
  ssize_t n = read(fd_.GetFd(), buf, sizeof(buf));
  if (n > 0) {
    read_buffer_.append(buf, n);
  } else if (n == 0) {
    int status;
    kill(pid_, SIGTERM);
    waitpid(pid_, &status, 0);
    pid_ = -1;

    result.remove_socket = true;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd_.GetFd(), NULL) == -1) {
      throw lib::exception::ResponseStatusException(
          lib::http::kInternalServerError);
    }

    int write_fd = input_write_fd_.GetFd();
    if (write_fd != -1) {
      epoll_ctl(epoll_fd, EPOLL_CTL_DEL, write_fd, NULL);
    }

    if (owner_) {
      if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        owner_->OnCgiExecutionFinished(epoll_fd, read_buffer_);
      } else {
        owner_->OnCgiExecutionError(epoll_fd);
      }
    }
  } else if (n == -1) {
    // EAGAIN: next epoll_wait will notify again
  }
}

void CgiSocket::HandleEpollOut(int epoll_fd, SocketResult& result) {
  if (write_buffer_.empty()) {
    if (input_write_fd_.GetFd() != -1) {
      epoll_ctl(epoll_fd, EPOLL_CTL_DEL, input_write_fd_.GetFd(), NULL);
      input_write_fd_.Reset();
    }
    return;
  }

  ssize_t n = write(input_write_fd_.GetFd(), write_buffer_.data(),
                    write_buffer_.size());
  if (n > 0) {
    write_buffer_.erase(0, n);
    if (write_buffer_.empty()) {
      epoll_ctl(epoll_fd, EPOLL_CTL_DEL, input_write_fd_.GetFd(), NULL);
      input_write_fd_.Reset();
    }
  } else if (n == 0) {
    if (owner_) {
      owner_->OnCgiExecutionError(epoll_fd);
    }
    result.remove_socket = true;
  } else if (n == -1) {
    // EAGAIN: next epoll_wait will notify again
  }
}

ssize_t CgiSocket::Send(const std::string& data) {
  write_buffer_ = data;
  return write_buffer_.size();
}

void CgiSocket::OnSetOwner(ClientSocket* owner) {
  owner_ = owner;
}

int CgiSocket::GetWriteFd() const {
  return input_write_fd_.GetFd();
}

void CgiSocket::CloseWriteFd() {
  input_write_fd_.Reset();
}
