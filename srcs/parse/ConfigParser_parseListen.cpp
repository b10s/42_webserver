#include "ConfigParser.hpp"

/*
The isValidHost() function should ensure that only 
- valid IPv4 addresses (0.0.0.0, 127.0.0.1, etc.)
- localhost
- syntactically acceptable domain names
This program must not crash or terminate unexpectedly,
so we need to reject malformed hosts to avoid socket binding errors.
The check can be lightweight; strict DNS validation isn’t required in Webserv.
*/
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

inline bool IsValidIPv4(const std::string& host) {
  std::istringstream ss(host);
  std::string segment;
  int seg_cnt = 0; // count of segments

  while (std::getline(ss, segment, '.')) {
    if (++seg_cnt > 4) return false;
    if (segment.empty() || segment.length() > 3) return false;
    for (size_t i = 0; i < segment.length(); ++i) {
      if (!std::isdigit(segment[i])) return false;
    }
    int value = std::atoi(segment.c_str());
    if (value < 0 || value > 255) return false;
    if (segment[0] == '0' && segment.length() > 1) return false; // leading zero is not allowed in IPv4
  }
  return seg_cnt == 4;
}

inline bool LooksLikeDomain(const std::string& host) {
  size_t dot_pos = host.find('.');
  if (dot_pos == std::string::npos || dot_pos == 0 || dot_pos == host.length() - 1) {
    return false; // No dot or dot at start/end
  }
  return true;
}

bool IsValidHost(const std::string& host) {
    if (host.empty()) return false;
    if (host == "localhost") return true;
    if (IsValidIPv4(host)) return true;
    if (ContainsInvalidChars(host)) return false;
    if (LooksLikeDomain(host)) return true;
    return false;
}


/*
"host:port" must be written without spaces (e.g., "127.0.0.1:8080").
Spaces around ':' are a syntax error.
Default values for port and host are set in the ServerConfig constructor:
  - Port defaults to 80
  - Host/address defaults to 0.0.0.0 (listen on all interfaces)
*/
void ConfigParser ::ParseListen(ServerConfig* server_config) {
  std::string token1 = Tokenize(content);
  if (token1.empty()) {
    throw std::runtime_error("Syntax error : expected host or port after listen");
  }
  std::string::size_type colon_pos = token1.find(':');
  if (colon_pos != std::string::npos) {
    std::string host = token1.substr(0, colon_pos);
    std::string port = token1.substr(colon_pos + 1);
    if (!IsValidHost(host))
      throw std::runtime_error(
          "Invalid host in listen directive: " + host);
    if (!IsValidPortNumber(port))
      throw std::runtime_error(
          "Invalid port number after ':' in listen directive: " + port);
    std::string end = Tokenize(content);
    if (end != ";")
      throw std::runtime_error(
          "Syntax error: expected ';' after listen directive " + end);
    server_config->SetListen(host, port);
    return;
  }
  // host only or port only
  std::string token2 = Tokenize(content);
  if (token2 != ";") {
    throw std::runtime_error("Syntax error: expected ';' after listen value: " +
                             token2);
  }
  if (IsAllDigits(token1)) {
    if (!IsValidPortNumber(token1))
      throw std::runtime_error("Invalid port number in listen directive: " +
                               token1);
    server_config->SetPort(token1);
  } else {
    if (!IsValidHost(token1))
      throw std::runtime_error("Invalid host in listen directive: " + token1);
    server_config->SetHost(token1);
  }
  return;
}



