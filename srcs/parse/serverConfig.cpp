#include "../../include/config.hpp"

ServerConfig::ServerConfig() : host_(), port_(), serverName_(), maxBodySize_(0) {}

void ServerConfig::setHost(const std::string &host) { host_ = host; }
void ServerConfig::setPort(const std::string &port) { port_ = port; }
void ServerConfig::setServerName(const std::string &serverName) { serverName_ = serverName; }
void ServerConfig::setMaxBodySize(int size) { maxBodySize_ = size; }