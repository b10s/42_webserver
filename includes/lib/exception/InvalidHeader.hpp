#ifndef LIB_EXCEPTION_INVALID_HEADER_HPP_
#define LIB_EXCEPTION_INVALID_HEADER_HPP_

#include <exception>

namespace lib {
namespace exception {
class InvalidHeader : public std::exception {
 public:
  virtual const char* what() const throw();
};
}  // namespace exception
}  // namespace lib

#endif
