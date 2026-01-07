#include "socket/CgiSocket.hpp"

#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>

#include "lib/type/Fd.hpp"
#include "socket/ClientSocket.hpp"

CgiSocket::CgiSocket(lib::type::Fd fd, int pid)
    : ASocket(fd), pid_(pid), owner_(NULL) {
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
  if (events & EPOLLIN) {
    char buf[kBufferSize];
    ssize_t n = read(fd_.GetFd(), buf, sizeof(buf));
    if (n > 0) {
      read_buffer_.append(buf, n);
    } else {
      int status;
      waitpid(pid_, &status, 0);
      pid_ = -1;

      result.remove_socket = true;
      epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd_.GetFd(), NULL);

      if (owner_) {
        owner_->OnCgiExecutionFinished(epoll_fd, read_buffer_);
      }
    }
  }
  return result;
}

ssize_t CgiSocket::Send(const std::string& data) {
  return write(fd_.GetFd(), data.c_str(), data.length());
}

void CgiSocket::OnSetOwner(ClientSocket* owner) {
  owner_ = owner;
}
