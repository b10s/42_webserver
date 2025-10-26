#include "ConfigParser.hpp"

void ConfigParser::parseRoot(Location* location) {
  parseSimpleDirective(location, &Location::setRoot,
                       "root directory path");
}

void ConfigParser::parseCgiPath(Location* location) {
  parseSimpleDirective(location, &Location::setCgiPath, "cgi_path value");
}

void ConfigParser::parseUploadPath(Location* location) {
  parseSimpleDirective(location, &Location::setUploadPath,
                       "upload_path value");
}

void ConfigParser::parseRedirect(Location* location) {
  parseSimpleDirective(location, &Location::setRedirect,
                       "redirect value");
}

void ConfigParser::parseServerName(ServerConfig* serverConfig) {
  parseSimpleDirective(serverConfig, &ServerConfig::setServerName,
                       "server_name value");
}
