#include "lib/exception/ConnectionClosed.hpp"

namespace lib {
namespace exception {

const char* ConnectionClosed::what() const throw() {
  return "Connection Closed";
}

}  // namespace exception
}  // namespace lib
