#include "Webserv.hpp"

#include <csignal>   // For signal(), SIGPIPE, SIG_IGN
#include <iostream>  // For std::cout, std::cerr

#include "lib/utils/string_utils.hpp"

Webserv::Webserv() {
  // Default constructor
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

  unsigned short port =
      lib::utils::StrToUnsignedShort(configs[0].GetPort()).Value();

  epoll_.CreateSocket();
  epoll_.SetServerAddr(port);
  epoll_.BindSocket();
  epoll_.ListenSocket();
  epoll_.CreateInstance();
  epoll_.AddSocketToInstance(epoll_.GetServerFd());
}

void Webserv::Run() {
  while (true) {
    int nfds = epoll_.Wait();
    if (nfds == -1) {
      // throw error;
    }

    for (int i = 0; i < nfds; ++i) {
      if (events_[i].data.fd == epoll_.GetServerFd()) {
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(epoll_.GetServerFd(), (sockaddr*)&client_addr,
                               &client_addr_len);
        if (client_fd == -1) {
          // throw error;
        }
        epoll_.AddSocketToInstance(client_fd);
      } else {
        int client_fd = events_[i].data.fd;
        char buffer[1024];
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
          // throw error;
        } else {
          HttpResponse res;
          res.SetStatus(200, "OK");
          res.AddHeader("Content-Type", "text/plain");
          res.SetBody("Hello, world\n");
          std::string res_str = res.ToString();
          send(client_fd, res_str.c_str(), res_str.length(), 0);
        }
      }
    }
  }
}

// creates one server instance per port configuration
// we must handle multiple ServerConfig instances but no virtual hosting
void Webserv::InitServersFromConfigs(const std::vector<ServerConfig>& configs) {
  port_to_server_configs_.clear();

  // Group configurations by port
  for (std::vector<ServerConfig>::const_iterator server = configs.begin();
       server != configs.end(); ++server) {
    const std::string& port = server->GetPort();
    if (port_to_server_configs_.find(port) != port_to_server_configs_.end()) {
      // Port already exists - warn or throw an error?
      std::cerr << "Warning: Multiple server blocks for port " << port
                << ", using first one only" << std::endl;
      continue;
    }
    port_to_server_configs_[port] = *server;
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
// void Webserv::HandleRequest(int client_fd, const std::string& port) {
//   (void)client_fd;  // suppress unused variable warning
//   const ServerConfig* server_config = FindServerConfigByPort(port);
//   if (server_config) {
//     std::cout << "Found server config for port " << port << std::endl;
//     const std::vector<Location>& locations = server_config->GetLocations();
//     if (!locations.empty()) {
//       std::cout << "  Root: " << locations.front().GetRoot() << std::endl;
//     } else {
//       std::cout << "  Root: (no locations defined)" << std::endl;
//     }
//     std::cout << "  MaxBodySize: " << server_config->GetMaxBodySize()
//               << std::endl;
//     std::cout << "  Host: " << server_config->GetHost() << std::endl;

//     // TODO: Process request with these settings...
//   } else {
//     std::cout << "ERROR: No server configuration found for port " << port
//               << std::endl;
//     // Handle error: no configuration for this port
//     // throw 500 Internal Server Error??
//   }
// }

// // for testing purposes
// void Webserv::TestConfiguration() {
//   std::cout << "\n=== Testing Webserver Configuration ===" << std::endl;

//   // 1. Check the total number of configurations
//   const std::map<std::string, ServerConfig>& configs = GetPortConfigs();
//   std::cout << "Total server configurations loaded: " << configs.size()
//             << std::endl;

//   if (configs.empty()) {
//     std::cout << "ERROR: No server configurations found!" << std::endl;
//     return;
//   }

//   // 2. Display each configuration
//   std::cout << "\nServer configurations:" << std::endl;
//   for (std::map<std::string, ServerConfig>::const_iterator it =
//   configs.begin();
//        it != configs.end(); ++it) {
//     const std::string& port = it->first;
//     const ServerConfig& config = it->second;

//     std::cout << "  Port " << port << ":" << std::endl;
//     std::cout << "    Host: " << config.GetHost() << std::endl;
//     std::cout << "    MaxBodySize: " << config.GetMaxBodySize() << std::endl;

//     // If locations exist
//     const std::vector<Location>& locations = config.GetLocations();
//     if (!locations.empty()) {
//       std::cout << "    Root: " << locations.front().GetRoot() << std::endl;
//     }
//     std::cout << std::endl;
//   }

//   // 3. Test HandleRequest
//   std::cout << "=== Testing HandleRequest ===" << std::endl;
//   for (std::map<std::string, ServerConfig>::const_iterator it =
//   configs.begin();
//        it != configs.end(); ++it) {
//     const std::string& port = it->first;

//     std::cout << "\nTesting HandleRequest for port " << port << ":"
//               << std::endl;
//     HandleRequest(999, port);  // 999はダミーのclient_fd
//   }

//   // 4. Test with non-existent port
//   std::cout << "\n=== Testing Invalid Port ===" << std::endl;
//   std::cout << "Testing HandleRequest for invalid port 9999:" << std::endl;
//   HandleRequest(999, "9999");

//   std::cout << "\n=== Test Complete ===" << std::endl;
// }
