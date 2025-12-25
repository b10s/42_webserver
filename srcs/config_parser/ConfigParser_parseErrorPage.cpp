#include "ConfigParser.hpp"

/*
Internal URI (e.g., /404.html, /error/default.html)
  Processed by nginx as an internal redirect within the same server.
External URL (scheme + host + path, e.g., http://example.com/404.html)
  Triggers an external redirect (e.g., 302) sent to the client.
  The browser is redirected to another domain or server.
Relative Path (e.g., ./404.html, ../404.html)
  Invalid in nginx configuration. Cannot be matched in nginx's routing
mechanism.
*/
void ConfigParser::ParseErrorPage(ServerConfig* server_config) {
  std::string token = Tokenize(content);
  if (token.empty()) {
    throw std::runtime_error("Syntax error : expected error code" + token);
  }
  std::string error_code_str = token;
  int error_code = std::atoi(token.c_str());
  if (error_code < 400 || error_code > 599) {
    throw std::runtime_error("Invalid error code: " + token);
  }

  token = Tokenize(content);
  if (token.empty()) {
    throw std::runtime_error("Syntax error : expected error page path" + token);
  }
  if (token[0] == '.') {
    throw std::runtime_error("Error page path must be absolute: " + token);
  }
  if (token.find(url_constants::kHttpsPrefix) != 0 &&
      token.find(url_constants::kHttpPrefix) != 0 && token[0] != '/') {
    token = "/" + token;
  }
  server_config->SetErrorPage(static_cast<lib::http::Status>(error_code),
                              token);
  ConsumeExpectedSemicolon("error_page");
}
