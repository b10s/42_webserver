#include "ConfigParser.hpp"

/*
If the port is omitted, the default port is 80.
If the address is omitted, the server listens on all addresses (0.0.0.0).
*/
ServerConfig::ServerConfig()
    : host_("0.0.0.0"),
      port_(80),
      server_name_(),
      max_body_size_(0),
      has_listen_(false),
      has_server_name_(false),
      has_max_body_(false) {
}

void ServerConfig::SetListen(const std::string& host,
                             const unsigned short& port) {
  if (has_listen_) {
    throw std::runtime_error("Duplicate listen directive");
  }
  host_ = host;
  port_ = port;
  has_listen_ = true;
}

void ServerConfig::SetHost(const std::string& host) {
  if (has_listen_) {
    throw std::runtime_error("Duplicate listen directive");
  }
  host_ = host;
  has_listen_ = true;
}

void ServerConfig::SetPort(const unsigned short& port) {
  if (has_listen_) {
    throw std::runtime_error("Duplicate listen directive");
  }
  port_ = port;
  has_listen_ = true;
}

void ServerConfig::SetServerName(const std::string& server_name) {
  if (has_server_name_) {
    throw std::runtime_error("Duplicate server_name directive");
  }
  server_name_ = server_name;
  has_server_name_ = true;
}

void ServerConfig::SetMaxBodySize(int size) {
  if (has_max_body_) {
    throw std::runtime_error("Duplicate client_max_body_size directive");
  }
  max_body_size_ = size;
  has_max_body_ = true;
}
