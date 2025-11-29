#include "lib/exception/InvalidHeader.hpp"

namespace lib {
namespace exception {

const char* InvalidHeader::what() const throw() {
  return "Invalid header: contains CR or LF characters";
}

}  // namespace exception
}  // namespace lib
