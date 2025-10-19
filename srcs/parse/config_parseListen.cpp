#include "config_parser.hpp"

/*
"host:port" must be written without spaces (e.g., "127.0.0.1:8080"). 
Spaces around ':' are a syntax error.
Default values for port and host are set in the ServerConfig constructor:
  - Port defaults to 80
  - Host/address defaults to 0.0.0.0 (listen on all interfaces)
*/
void ConfigParser ::parseListen(ServerConfig *serverConfig) {
  std::string token1 = tokenize(content_);
  if (token1.empty()) {
    throw std::runtime_error("Syntax error : expected host or port after listen");
  }
  std::string::size_type colon_pos = token1.find(':');
  if (colon_pos != std::string::npos) {
    std::string host = token1.substr(0, colon_pos);
    std::string port = token1.substr(colon_pos + 1);
    if (!isValidPortNumber(port))
      throw std::runtime_error("Invalid port number after ':' in listen directive: " + port);
    std::string end = tokenize(content_);
    if (end != ";")
      throw std::runtime_error("Syntax error: expected ';' after listen directive " + end);
    serverConfig->setHost(host);
    serverConfig->setPort(port);
    return ;
  }
  // host only or port only
  std::string token2 = tokenize(content_);
  if (token2 != ";") {
    throw std::runtime_error("Syntax error: expected ';' after listen value: " + token2);
  }
  if (isAllDigits(token1)) {
    if (!isValidPortNumber(token1))
      throw std::runtime_error("Invalid port number in listen directive: " + token1);
    serverConfig->setPort(token1);
  } else {
    serverConfig->setHost(token1);
  }
  return ;
}

// TODO: We should not allow hostnames like "http;".
// Introduce a validation function (e.g., isValidHost())
// to check whether the given hostname is valid (IPv4, localhost, or domain name).