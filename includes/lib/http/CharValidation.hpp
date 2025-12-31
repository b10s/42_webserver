#ifndef LIB_HTTP_CHARVALIDATION_HPP_
#define LIB_HTTP_CHARVALIDATION_HPP_

namespace lib {
namespace http {

// for ConsumeHeader
inline bool IsValidHeaderChar(char c) {
  unsigned char uc = static_cast<unsigned char>(c);
  return uc >= ' ' && uc <= '~';  // printable ASCII (32-126) only
}

// for URI and query parameters, used in Index file parsing as well
inline bool IsVisibleAscii(char c) {
  return c >= '!' && c <= '~';  // 33-126 only
}

}  // namespace http
}  // namespace lib

#endif
