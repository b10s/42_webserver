#ifndef HOST_VALIDATION_HPP_
#define HOST_VALIDATION_HPP_

#include <string>
#include <sstream>
#include <cctype>

namespace host_validation {

inline bool IsHostChar(char c) {
  return std::isalnum(c) || c == '-' || c == '.';
}

inline bool ContainsInvalidChars(const std::string& host) {
  for (size_t i = 0; i < host.length(); ++i) {
    if (!IsHostChar(host[i])) {
      return true;
    }
  }
  return false;
}

bool LooksLikeIPv4(const std::string& host);
bool IsValidIPv4Segment(const std::string& segment);
bool IsValidIPv4(const std::string& host);
bool LooksLikeDomain(const std::string& host);
bool IsValidHost(const std::string& host);

} // namespace host_validation

#endif // HOST_VALIDATION_HPP_
