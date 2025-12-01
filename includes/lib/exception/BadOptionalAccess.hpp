#ifndef LIB_EXCEPTION_BAD_OPTIONAL_ACCESS_HPP_
#define LIB_EXCEPTION_BAD_OPTIONAL_ACCESS_HPP_

#include <exception>

namespace lib {
namespace exception {
class BadOptionalAccess : public std::exception {
 public:
  const char* what() const throw();
};
}  // namespace exception
}  // namespace lib

#endif
