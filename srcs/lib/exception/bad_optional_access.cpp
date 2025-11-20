#include "lib/exception/bad_optional_access.hpp"

#include "lib/exception/messages.hpp"

namespace lib {
namespace exception {
const char* BadOptionalAccess::what() const throw() {
  return k_bad_optional_access_msg;
}
}  // namespace exception
}  // namespace lib
