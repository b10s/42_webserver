#include "config_parser.hpp"

void ConfigParser::parseRoot(Location* location) {
    parseSingleValue(this, location, &Location::setRoot, "root directory path");
}

void ConfigParser::parseCgiPath(Location* location) {
    parseSingleValue(this, location, &Location::setCgiPath, "cgi_path value");
}

void ConfigParser::parseUploadPath(Location* location) {
    parseSingleValue(this, location, &Location::setUploadPath, "upload_path value");
}

void ConfigParser::parseRedirect(Location* location) {
    parseSingleValue(this, location, &Location::setRedirect, "redirect value");
}

void ConfigParser::parseServerName(ServerConfig* serverConfig) {
    parseSingleValue(this, serverConfig, &ServerConfig::setServerName, "server_name value");
}
