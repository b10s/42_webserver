#include "lib/exception/ResponseStatusException.hpp"

namespace lib {
namespace exception {

ResponseStatusException::ResponseStatusException(lib::http::Status status)
    : std::runtime_error(lib::http::StatusToString(status)), status_(status) {
}

lib::http::Status lib::exception::ResponseStatusException::GetStatus() const {
  return status_;
}

}  // namespace exception
}  // namespace lib
