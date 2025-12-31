#include "lib/utils/string_utils.hpp"

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "lib/type/Optional.hpp"

namespace lib {
namespace utils {

std::string ToLowerAscii(const std::string& s) {
  std::string result = s;
  for (size_t i = 0; i < result.size(); ++i) {
    result[i] =
        static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
  }
  return result;
}

lib::type::Optional<long> StrToLong(const std::string& s) {
  std::stringstream ss(s);
  long result;
  ss >> result;
  if (ss.fail() || !ss.eof()) {
    return lib::type::Optional<long>();
  }
  return lib::type::Optional<long>(result);
}

lib::type::Optional<unsigned short> StrToUnsignedShort(const std::string& s) {
  std::stringstream ss(s);
  unsigned short result;
  ss >> result;
  if (ss.fail() || !ss.eof()) {
    return lib::type::Optional<unsigned short>();
  }
  return lib::type::Optional<unsigned short>(result);
}

bool StartsWith(const std::string& str, const std::string& prefix) {
  if (prefix.size() > str.size()) {
    return false;
  }
  return str.compare(0, prefix.size(), prefix) == 0;
}

lib::type::Optional<std::string> GetFirstToken(const std::string& str,
                                               const std::string& delimiter) {
  std::basic_string<char>::size_type pos = str.find(delimiter);
  if (pos == std::string::npos) return lib::type::Optional<std::string>();
  return lib::type::Optional<std::string>(str.substr(0, pos));
}

}  // namespace utils
}  // namespace lib
