#ifndef LIB_EXCEPTION_CONNECTION_CLOSED_HPP_
#define LIB_EXCEPTION_CONNECTION_CLOSED_HPP_

#include <exception>

namespace lib {
namespace exception {

class ConnectionClosed : public std::exception {
 public:
  virtual const char* what() const throw();
};

}  // namespace exception
}  // namespace lib

#endif
