#include "host_validation.hpp"
#include <sstream>
/*
The IsValidHost() function should ensure that only
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
  // check that host contains only digits and dots (disallow hyphens)
  for (size_t i = 0; i < host.length(); ++i) {
    char c = host[i];
    if (!std::isdigit(c) && c != '.') {
      return false;
    }
  }
  // then split by '.' and validate each segment
  std::istringstream ss(host);
  std::string segment;
  int seg_cnt = 0;  // count of segments
  while (std::getline(ss, segment, '.')) {
    if (++seg_cnt > 4) return false;
    if (!IsValidIPv4Segment(segment)) return false;
  }
  return seg_cnt == 4;
}

bool host_validation::IsValidDomainName(const std::string& host) {
  if (host.empty() || host[0] == '.' || host[host.length() - 1] == '.') {
    return false;  // empty or starts/ends with dot
  }
  std::istringstream ss(host);
  std::string label;
  int label_count = 0;
  while (std::getline(ss, label, '.')) {
    if (!IsValidDomainLabel(label)) return false;
    ++label_count;
  }
  return label_count >= 2;  // at least two labels (e.g., example.com)
}

// Check for leading/trailing hyphens in each label
// Most DNS resolvers will reject hostnames starting with hyphens as invalid
// RFC 1123 explicitly forbids leading/trailing hyphens in each label
bool host_validation::IsValidDomainLabel(const std::string& label) {
  if (label.empty() || label.length() > 63) return false;
  if (label[0] == '-' || label[label.length() - 1] == '-') {
    return false;
  }
  for (std::size_t i = 0; i < label.length(); ++i) {
    char c = label[i];
    if (!std::isalnum(c) && c != '-') return false;
  }
  return true;
}

bool host_validation::IsValidHost(const std::string& host) {
  if (host.empty()) return false;
  if (host == "localhost") return true;
  // Check for IPv4
  // looks redundant but necessary to avoid
  // misclassifying invalid IPV4 as domain names
  if (LooksLikeIPv4(host)) {
    if (IsValidIPv4(host)) return true;
    return false;
  }
  // Check for Domain Name
  if (ContainsInvalidChars(host)) return false;
  return IsValidDomainName(host);
}
