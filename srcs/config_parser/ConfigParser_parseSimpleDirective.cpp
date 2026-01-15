#include "ConfigParser.hpp"
#include "ServerConfig.hpp"

void ConfigParser::ParseRedirect(Location* location) {
  ParseSimpleDirective(location, &Location::SetRedirect, "redirect value");
}

void ConfigParser::ParseCgi(Location* location) {
  ParseSimpleDirective(location, &Location::SetCgiEnabled, "cgi value");
}

void ConfigParser::ParseServerName(ServerConfig* server_config) {
  ParseSimpleDirective(server_config, &ServerConfig::SetServerName,
                       "server_name value");
}
