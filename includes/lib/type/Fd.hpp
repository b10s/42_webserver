#ifndef FD_HPP_
#define FD_HPP_

namespace lib {
namespace type {

class Fd {
 private:
  int fd_;

 public:
  Fd();
  explicit Fd(int fd);
  ~Fd();

  void Reset(int new_fd = -1);
  int GetFd() const;

  Fd(const Fd& other);
  Fd& operator=(const Fd& other);
};

}  // namespace type
}  // namespace lib

#endif  // FD_HPP_
