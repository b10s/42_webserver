#include "socket/CgiSocket.hpp"

#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>

#include "lib/exception/ResponseStatusException.hpp"
#include "lib/type/Fd.hpp"
#include "lib/utils/file_utils.hpp"
#include "socket/ClientSocket.hpp"

CgiSocket::CgiSocket(lib::type::Fd fd, int pid)
    : ASocket(fd), pid_(pid), owner_(NULL), write_done_(true) {
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
    if (events & EPOLLIN) {
      HandleEpollIn(epoll_fd, result);
    }
    if (events & EPOLLOUT) {
      HandleEpollOut(epoll_fd, result);
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
  if (!write_done_) {
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT;
    ev.data.ptr = this;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd_.GetFd(), &ev);
    return;
  }
  char buf[kBufferSize];
  ssize_t n = read(fd_.GetFd(), buf, sizeof(buf));
  if (n > 0) {
    read_buffer_.append(buf, n);
  } else if (n == 0) {
    int status;
    waitpid(pid_, &status, 0);
    pid_ = -1;

    result.remove_socket = true;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd_.GetFd(), NULL) == -1) {
      int saved_errno = errno;
      throw lib::exception::ResponseStatusException(
          lib::utils::MapErrnoToHttpStatus(saved_errno));
    }

    if (owner_) {
      if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        owner_->OnCgiExecutionFinished(epoll_fd, read_buffer_);
      } else {
        owner_->OnCgiExecutionError(epoll_fd);
      }
    }
  }
}

void CgiSocket::HandleEpollOut(int epoll_fd, SocketResult& result) {
  if (write_buffer_.empty()) return;

  ssize_t n = write(fd_.GetFd(), write_buffer_.data(), write_buffer_.size());
  if (n > 0) {
    write_buffer_.erase(0, n);
    if (write_buffer_.empty()) {
      write_done_ = true;
      epoll_event ev;
      ev.events = EPOLLIN;
      ev.data.ptr = this;
      epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd_.GetFd(), &ev);
    }
  } else if (n == -1 && errno != EAGAIN) {
    if (owner_) {
      owner_->OnCgiExecutionError(epoll_fd);
    }
    result.remove_socket = true;
  }
}

ssize_t CgiSocket::Send(const std::string& data) {
  write_buffer_ = data;
  write_done_ = false;
  return write_buffer_.size();
}

void CgiSocket::OnSetOwner(ClientSocket* owner) {
  owner_ = owner;
}
