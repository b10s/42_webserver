#include "ConfigParser.hpp"

/*
"host:port" must be written without spaces (e.g., "127.0.0.1:8080").
Spaces around ':' are a syntax error.
Default values for port and host are set in the ServerConfig constructor:
  - Port defaults to 80
  - Host/address defaults to 0.0.0.0 (listen on all interfaces)
*/
void ConfigParser ::ParseListen(ServerConfig* server_config) {
  std::string token1 = Tokenize(content_);
  if (token1.empty()) {
    throw std::runtime_error(
        "Syntax error : expected host or port after listen");
  }
  std::string::size_type colon_pos = token1.find(':');
  if (colon_pos != std::string::npos) {
    std::string host = token1.substr(0, colon_pos);
    std::string port = token1.substr(colon_pos + 1);
    if (!IsValidPortNumber(port))
      throw std::runtime_error(
          "Invalid port number after ':' in listen directive: " + port);
    std::string end = Tokenize(content_);
    if (end != ";")
      throw std::runtime_error(
          "Syntax error: expected ';' after listen directive " + end);
    server_config->SetHost(host);
    server_config->SetPort(port);
    return;
  }
  // host only or port only
  std::string token2 = Tokenize(content_);
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
    server_config->SetHost(token1);
  }
  return;
}

// TODO: We should not allow hostnames like "http;".
// Introduce a validation function (e.g., isValidHost())
// to check whether the given hostname is valid (IPv4, localhost, or domain
// name).
