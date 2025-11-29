#include "lib/exception/InvalidHeader.hpp"

#include "lib/exception/messages.hpp"

namespace lib {
namespace exception {
const char* InvalidHeader::what() const throw() {
  return k_invalid_header_msg;
}
}  // namespace exception
}  // namespace lib
