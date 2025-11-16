#include "ConfigParser.hpp"

void ConfigParser::ParseRoot(Location* location) {
  ParseSimpleDirective(location, &Location::SetRoot, "root directory path");
}

void ConfigParser::ParseCgiPath(Location* location) {
  ParseSimpleDirective(location, &Location::SetCgiPath, "cgi_path value");
}

void ConfigParser::ParseUploadPath(Location* location) {
  ParseSimpleDirective(location, &Location::SetUploadPath, "upload_path value");
}

void ConfigParser::ParseRedirect(Location* location) {
  ParseSimpleDirective(location, &Location::SetRedirect, "redirect value");
}

void ConfigParser::ParseServerName(ServerConfig* server_config) {
  ParseSimpleDirective(server_config, &ServerConfig::SetServerName,
                       "server_name value");
}
