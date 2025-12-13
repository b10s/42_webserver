#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "ConfigParser.hpp"
#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class Webserv {
 private:
  std::vector<ServerConfig> servers_;

 public:
  vector<ServerConfig> servers_;
  Webserv();
  Webserv(const ConfigParser& config_file);
  ~Webserv();

  void Run();
};

#endif // WEBSERV_HPP
