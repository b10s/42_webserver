#include "config_parser.hpp"

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
void ConfigParser::parseErrorPage(ServerConfig* serverConfig) {
  std::string token = tokenize(content_);
  if (token.empty()) {
    throw std::runtime_error("Syntax error : expected error code" + token);
  }
  std::string errorCodeStr = token;
  int errorCode = std::atoi(token.c_str());
  if (errorCode < 400 || errorCode > 599) {
    throw std::runtime_error("Invalid error code: " + token);
  }

  token = tokenize(content_);
  if (token.empty()) {
    throw std::runtime_error("Syntax error : expected error page path" + token);
  }
  if (token[0] == '.') {
    throw std::runtime_error("Error page path must be absolute: " + token);
  }
  if (token.find(UrlConstants::kHttpsPrefix) != 0 &&
      token.find(UrlConstants::kHttpPrefix) != 0 && token[0] != '/') {
    token = "/" + token;
  }
  serverConfig->setErrorPage(static_cast<HttpStatus>(errorCode), token);
  token = tokenize(content_);
  if (token != ";") {
    throw std::runtime_error(
        "Syntax error: expected ';' after error_page directive" + token);
  }
}
