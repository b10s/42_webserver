#include "Webserv.hpp"

Webserv::Webserv(const std::string& config_file) {
  signal(SIGPIPE, SIG_IGN);  // avoid client disconnect crashes

  ConfigParser config_parser;
  config_parser.LoadFile(config_file);
  config_parser.Parse();
  const std::vector<ServerConfig>& configs =
      config_parser.GetServerConfigs();
  initServersFromConfigs(configs);

  // TODO: Initialize sockets and other server setup here
}

// creates one server instance per port configuration
// we must handle multiple ServerConfig instances but no virtual hosting
void Webserv::initServersFromConfigs(const std::vector<ServerConfig>& configs) {
  port_to_server_configs_.clear();

  // Group configurations by port
  for (std::vector<ServerConfig>::const_iterator it = configs.begin();
       it != configs.end(); ++it) {
    const std::string& port = it->GetPort();
    if (port_to_server_configs_.find(port) != port_to_server_configs_.end()) {
      // Port already exists - warn or throw an error?
      std::cerr << "Warning: Multiple server blocks for port " << port 
                << ", using first one only" << std::endl;
      continue;
    }
    port_to_server_configs_[port] = *it;
  }
}

const std::map<std::string, ServerConfig>& Webserv::GetPortConfigs() const {
  return port_to_server_configs_;
}

const ServerConfig* Webserv::FindServerConfigByPort(const std::string& port) const {
  std::map<std::string, ServerConfig>::const_iterator it =
      port_to_server_configs_.find(port);
  if (it != port_to_server_configs_.end()) {
    return &it->second;
  }
  return nullptr;  // Not found
}

void Webserv::HandleRequest(int client_fd, const std::string& port) {
    const ServerConfig* server_config = FindServerConfigByPort(port);
    if (server_config) {
        // Process the request using server_config
    } else {
        // Handle error: no configuration for this port
        // throw 500 Internal Server Error??
        std::cerr << "Error: No server configuration found for port " << port << std::endl;
    }
    size_t max_body_size = server_config->GetMaxBodySize();

    // TODO: Process request with these settings...
}