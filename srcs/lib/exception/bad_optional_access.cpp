#include "lib/exception/bad_optional_access.hpp"

#include "lib/exception/messages.hpp"

namespace lib {
namespace exception {
const char* bad_optional_access::what() const throw() {
  return kBadOptionalAccessMsg;
}
}  // namespace exception
}  // namespace lib
