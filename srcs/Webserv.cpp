#include "Webserv.hpp"

#include <csignal>   // For signal(), SIGPIPE, SIG_IGN
#include <iostream>  // For std::cout, std::cerr

Webserv::Webserv() {
  // Default constructor (could initialize defaults if needed)
}

Webserv::~Webserv() {
  // Add any cleanup code here if needed
}

Webserv::Webserv(const std::string& config_file) {
  signal(SIGPIPE, SIG_IGN);  // avoid client disconnect crashes

  ConfigParser config_parser;
  config_parser.LoadFile(config_file);
  config_parser.Parse();
  const std::vector<ServerConfig>& configs = config_parser.GetServerConfigs();
  InitServersFromConfigs(configs);

  // TODO: Initialize sockets and other server setup here
}

// creates one server instance per port configuration
// we must handle multiple ServerConfig instances but no virtual hosting
void Webserv::InitServersFromConfigs(const std::vector<ServerConfig>& configs) {
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

const ServerConfig* Webserv::FindServerConfigByPort(
    const std::string& port) const {
  std::map<std::string, ServerConfig>::const_iterator it =
      port_to_server_configs_.find(port);
  if (it != port_to_server_configs_.end()) {
    return &it->second;
  }
  return NULL;  // Not found
}

// for debugging purposes
void Webserv::HandleRequest(int client_fd, const std::string& port) {
  (void)client_fd;  // suppress unused variable warning
  const ServerConfig* server_config = FindServerConfigByPort(port);
  if (server_config) {
    std::cout << "Found server config for port " << port << std::endl;
    const std::vector<LocationConfig>& locations = server_config->GetLocations();
    if (!locations.empty()) {
      std::cout << "  Root: " << locations.front().GetRoot() << std::endl;
    } else {
      std::cout << "  Root: (no locations defined)" << std::endl;
    }
    std::cout << "  MaxBodySize: " << server_config->GetMaxBodySize()
              << std::endl;
    std::cout << "  Host: " << server_config->GetHost() << std::endl;

    // TODO: Process request with these settings...
  } else {
    std::cout << "ERROR: No server configuration found for port " << port
              << std::endl;
    // Handle error: no configuration for this port
    // throw 500 Internal Server Error??
  }
}

// for testing purposes
void Webserv::TestConfiguration() {
  std::cout << "\n=== Testing Webserver Configuration ===" << std::endl;

  // 1. Check the total number of configurations
  const std::map<std::string, ServerConfig>& configs = GetPortConfigs();
  std::cout << "Total server configurations loaded: " << configs.size()
            << std::endl;

  if (configs.empty()) {
    std::cout << "ERROR: No server configurations found!" << std::endl;
    return;
  }

  // 2. 各設定を表示
  std::cout << "\nServer configurations:" << std::endl;
  for (std::map<std::string, ServerConfig>::const_iterator it = configs.begin();
       it != configs.end(); ++it) {
    const std::string& port = it->first;
    const ServerConfig& config = it->second;

    std::cout << "  Port " << port << ":" << std::endl;
    std::cout << "    Host: " << config.GetHost() << std::endl;
    std::cout << "    MaxBodySize: " << config.GetMaxBodySize() << std::endl;

    // Locationがある場合
    const std::vector<Location>& locations = config.GetLocations();
    if (!locations.empty()) {
      std::cout << "    Root: " << locations.front().GetRoot() << std::endl;
    }
    std::cout << std::endl;
  }

  // 3. HandleRequestをテスト
  std::cout << "=== Testing HandleRequest ===" << std::endl;
  for (std::map<std::string, ServerConfig>::const_iterator it = configs.begin();
       it != configs.end(); ++it) {
    const std::string& port = it->first;

    std::cout << "\nTesting HandleRequest for port " << port << ":"
              << std::endl;
    HandleRequest(999, port);  // 999はダミーのclient_fd
  }

  // 4. 存在しないポートのテスト
  std::cout << "\n=== Testing Invalid Port ===" << std::endl;
  std::cout << "Testing HandleRequest for invalid port 9999:" << std::endl;
  HandleRequest(999, "9999");

  std::cout << "\n=== Test Complete ===" << std::endl;
}
