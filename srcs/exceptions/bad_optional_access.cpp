#include "exceptions/bad_optional_access.hpp"

const char* bad_optional_access::what() const throw() {
  return "bad optional access";
}
