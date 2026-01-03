#ifndef LIB_UTILS_STRING_UTILS_HPP_
#define LIB_UTILS_STRING_UTILS_HPP_

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "lib/type/Optional.hpp"

namespace lib {
namespace utils {

std::string ToLowerAscii(const std::string& s);
lib::type::Optional<long> StrToLong(const std::string& s);
lib::type::Optional<unsigned short> StrToUnsignedShort(const std::string& s);
bool StartsWith(const std::string& str, const std::string& prefix);

template <typename T>
std::string ToString(const T& value) {
  std::ostringstream oss;
  oss << value;
  return oss.str();
}

lib::type::Optional<std::string> GetFirstToken(const std::string& str,
                                               const std::string& delimiter);

}  // namespace utils
}  // namespace lib

#endif
