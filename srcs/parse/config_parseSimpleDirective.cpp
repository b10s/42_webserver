#include "config_parser.hpp"

void ConfigParser::parseRoot(Location* location) {
  parseSimpleDirective(this, location, &Location::setRoot,
                       "root directory path");
}

void ConfigParser::parseCgiPath(Location* location) {
  parseSimpleDirective(this, location, &Location::setCgiPath, "cgi_path value");
}

void ConfigParser::parseUploadPath(Location* location) {
  parseSimpleDirective(this, location, &Location::setUploadPath,
                       "upload_path value");
}

void ConfigParser::parseRedirect(Location* location) {
  parseSimpleDirective(this, location, &Location::setRedirect,
                       "redirect value");
}

void ConfigParser::parseServerName(ServerConfig* serverConfig) {
  parseSimpleDirective(this, serverConfig, &ServerConfig::setServerName,
                       "server_name value");
}
