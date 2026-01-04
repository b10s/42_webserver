#include "ConfigParser.hpp"
#include "ServerConfig.hpp"

// TODO: normalize path
// root = NormalizeSlashes(root);
// root = TrimTrailingSlashExceptRoot(root);
void ConfigParser::ParseRoot(Location* location) {
  ParseSimpleDirective(location, &Location::SetRoot, "root directory path");
}

void ConfigParser::ParseUploadPath(Location* location) {
  ParseSimpleDirective(location, &Location::SetUploadPath, "upload_path value");
}

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
