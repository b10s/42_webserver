#ifndef LIB_HTTP_METHOD_HPP_
#define LIB_HTTP_METHOD_HPP_

#include <string>

namespace lib {
namespace http {

enum Method { kNone, kGet, kHead, kPost, kDelete, kUnknownMethod };

std::string MethodToString(Method method);

}  // namespace http
}  // namespace lib

#endif  // LIB_HTTP_METHOD_HPP_
