#include "config_parser.hpp"

/*
If the port is omitted, the default port is 80.
If the address is omitted, the server listens on all addresses (0.0.0.0).
*/
ServerConfig::ServerConfig()
    : host_("0.0.0.0"), port_("80"), serverName_(), maxBodySize_(0) {}

void ServerConfig::setHost(const std::string& host) { host_ = host; }
void ServerConfig::setPort(const std::string& port) { port_ = port; }
void ServerConfig::setServerName(const std::string& serverName) {
  serverName_ = serverName;
}
void ServerConfig::setMaxBodySize(int size) { maxBodySize_ = size; }