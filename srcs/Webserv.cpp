#include "Webserv.hpp"

#include <csignal>   // For signal(), SIGPIPE, SIG_IGN
#include <iostream>  // For std::cout, std::cerr
#include <stdexcept>
#include <string>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "RequestHandler.hpp"
#include "ServerConfig.hpp"
#include "lib/http/Method.hpp"
#include "lib/http/Status.hpp"
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

  epoll_.CreateInstance();

  for (std::map<unsigned short, ServerConfig>::const_iterator it =
           port_to_server_configs_.begin();
       it != port_to_server_configs_.end(); ++it) {
    unsigned short port_num = it->first;
    epoll_.AddServer(port_num);
  }
}

void Webserv::Run() {
  while (true) {
    int nfds = epoll_.Wait();
    epoll_event* events = epoll_.GetEvents();

    for (int i = 0; i < nfds; ++i) {
      if (epoll_.IsServerFd(events[i].data.fd)) {
        // server fd
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(events[i].data.fd, (sockaddr*)&client_addr,
                               &client_addr_len);
        if (client_fd == -1) {
          std::cerr << "Failed to accept fd:" << client_fd << std::endl;
          continue;
        }
        epoll_.Addsocket(client_fd);
      } else {
        // client fd
        if (events[i].events & EPOLLIN) {
          HandleEpollIn(events[i].data.fd);
        }
        if (events[i].events & EPOLLOUT) {
          HandleEpollOut(events[i].data.fd);
        }
      }
    }
  }
}

void Webserv::HandleEpollIn(int fd) {
  char buffer[kBufferSize];
  ssize_t bytes_received = recv(fd, buffer, sizeof(buffer), 0);

  if (bytes_received <= 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) return;
    epoll_.RemoveSocket(fd);
    close(fd);
    output_buffers_.erase(fd);
    requests_.erase(fd);
  } else {
    HttpRequest& req = requests_[fd];
    try {
      req.ParseRequest(buffer, bytes_received);
      if (req.IsDone()) {
        const ServerConfig* conf = FindServerConfigByPort(req.GetHostPort());
        if (conf == NULL) return;
        RequestHandler handler(*conf, req);
        HttpResponse res = response_[fd];
        res = handler.Run();
        output_buffers_[fd] = res.ToHttpString();
        epoll_.ModSocket(fd, EPOLLOUT);
        requests_.erase(fd);
      }
    } catch (const std::exception& e) {
      std::cerr << "Request handling error for fd " << fd << ": " << e.what()
                << std::endl;
      // TODO: Send an appropriate error response to the client
      requests_.erase(fd);
      epoll_.RemoveSocket(fd);
      close(fd);
    }
  }
}

void Webserv::HandleEpollOut(int fd) {
  if (output_buffers_.count(fd)) {
    std::string& buffer = output_buffers_[fd];
    ssize_t bytes_sent = send(fd, buffer.c_str(), buffer.length(), 0);

    if (bytes_sent == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) return;
      epoll_.RemoveSocket(fd);
      close(fd);
      output_buffers_.erase(fd);
      requests_.erase(fd);
    } else if (static_cast<size_t>(bytes_sent) < buffer.length()) {
      buffer = buffer.substr(bytes_sent);
    } else {
      output_buffers_.erase(fd);
      epoll_.RemoveSocket(fd);
      close(fd);
      requests_.erase(fd);
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
    const unsigned short& port = server->GetPort();
    if (port_to_server_configs_.find(port) != port_to_server_configs_.end()) {
      // Port already exists - warn or throw an error?
      std::cerr << "Warning: Multiple server blocks for port " << port
                << ", using first one only" << std::endl;
      continue;
    }
    port_to_server_configs_[port] = *server;
  }
}

const std::map<unsigned short, ServerConfig>& Webserv::GetPortConfigs() const {
  return port_to_server_configs_;
}

const ServerConfig* Webserv::FindServerConfigByPort(
    const unsigned short& port) const {
  std::map<unsigned short, ServerConfig>::const_iterator it =
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
