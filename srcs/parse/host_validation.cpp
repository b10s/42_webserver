#include "host_validation.hpp"

/*
The isValidHost() function should ensure that only
- valid IPv4 addresses (0.0.0.0, 127.0.0.1, etc.)
- localhost
- syntactically acceptable domain names
This program must not crash or terminate unexpectedly,
so we need to reject malformed hosts to avoid socket binding errors.
The check can be lightweight; strict DNS validation isn’t required in Webserv.
*/

bool host_validation::LooksLikeIPv4(const std::string& host) {
  for (size_t i = 0; i < host.length(); ++i) {
    if (!std::isdigit(host[i]) && host[i] != '.' && host[i] != '-') {
      return false;
    }
  }
  return true;
}

bool host_validation::IsValidIPv4Segment(const std::string& segment) {
  if (segment.empty() || segment.length() > 3) return false;
  if (segment.length() > 1 && segment[0] == '0')
    return false;  // leading zero is not allowed
  int value = 0;
  for (size_t i = 0; i < segment.length(); ++i) {
    unsigned char c = static_cast<unsigned char>(segment[i]);
    if (!std::isdigit(c)) {
      return false;
    }
    value = value * 10 + (c - '0');
  }
  return value >= 0 && value <= 255;
}

bool host_validation::IsValidIPv4(const std::string& host) {
  std::istringstream ss(host);
  std::string segment;
  int seg_cnt = 0;  // count of segments

  while (std::getline(ss, segment, '.')) {
    if (++seg_cnt > 4) return false;
    if (!IsValidIPv4Segment(segment)) return false;
  }
  return seg_cnt == 4;
}

bool host_validation::LooksLikeDomain(const std::string& host) {
  size_t dot_pos = host.find('.');
  if (dot_pos == std::string::npos || dot_pos == 0 ||
      dot_pos == host.length() - 1) {
    return false;  // No dot or dot at start/end
  }
  // Check for leading/trailing hyphens
  // Most DNS resolvers will reject hostnames starting with hyphens as invalid
  // RFC 1123 explicitly forbids leading/trailing hyphens in hostnames
  if (host[0] == '-' || host[host.length() - 1] == '-') {
    return false;
  }
  return true;
}

bool host_validation::IsValidHost(const std::string& host) {
  if (host.empty()) return false;
  if (host == "localhost") return true;
  if (LooksLikeIPv4(host)) {
    if (IsValidIPv4(host)) return true;
    return false;
  }
  if (ContainsInvalidChars(host)) return false;
  if (LooksLikeDomain(host)) return true;
  return false;
}
