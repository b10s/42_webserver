#ifndef LIB_EXCEPTION_RESPONSE_STATUS_EXCEPTION_HPP_
#define LIB_EXCEPTION_RESPONSE_STATUS_EXCEPTION_HPP_

#include <stdexcept>

#include "lib/http/Status.hpp"

namespace lib {
namespace exception {

class ResponseStatusException : public std::runtime_error {
 private:
  lib::http::Status status_;

 public:
  ResponseStatusException(lib::http::Status status);
  lib::http::Status GetStatus() const;
};

}  // namespace exception
}  // namespace lib

#endif  // LIB_EXCEPTION_RESPONSE_STATUS_EXCEPTION_HPP_
