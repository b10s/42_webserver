#include "lib/http/Method.hpp"

namespace lib {
namespace http {

std::string MethodToString(Method method) {
  switch (method) {
    case kGet:
      return "GET";
    case kHead:
      return "HEAD";
    case kPost:
      return "POST";
    case kDelete:
      return "DELETE";
    case lib::http::kUnknownMethod:
      return "UNKNOWN_METHOD";
    default:
      return "kNone";
  }
}

}  // namespace http
}  // namespace lib
