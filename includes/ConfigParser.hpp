#ifndef CONFIG_PARSER_HPP_
#define CONFIG_PARSER_HPP_

#include <sys/stat.h>

#include <cctype>  // for std::isalnum, std::isprint
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Location.hpp"
#include "ServerConfig.hpp"
#include "enums.hpp"

namespace config_tokens {
const std::string kListen = "listen";
const std::string kServerName = "server_name";
const std::string kMaxBody = "client_max_body_size";
const std::string kErrorPage = "error_page";
const std::string kLocation = "location";
const std::string kAllowMethods = "allow_methods";
const std::string kRoot = "root";
const std::string kAutoIndex = "autoindex";
const std::string kIndex = "index";
const std::string kUploadPath = "upload_path";
const std::string kServer = "server";
const std::string kRedirect = "return";
const std::string kCgi = "cgi";
}  // namespace config_tokens

namespace url_constants {
const std::string kHttpsPrefix = "https://";
const std::string kHttpPrefix = "http://";
}  // namespace url_constants

class ConfigParser {
 private:
  size_t current_pos_;
  std::vector<ServerConfig> server_configs_;
  bool IsValidPortNumber(const std::string& port) const;
  bool IsAllDigits(const std::string& str) const;
  bool IsDirective(const std::string& token) const;
  TokenType ToTokenType(const std::string& token) const;
  std::string Tokenize(const std::string& content);
  void ConsumeExpectedSemicolon(const std::string& directive_name);

 public:
  std::string content;  // Made public for easier access in parsing functions
  ConfigParser();       // Default constructor for tests
  explicit ConfigParser(const std::string& text);
  ~ConfigParser();
  void LoadFileOrThrowRuntime(const std::string& filename);

  void Parse();
  void ParseServer();
  void ParseListen(ServerConfig* server_config);
  void ParseServerName(ServerConfig* server_config);
  void ParseMaxBody(ServerConfig* server_config);
  void ParseErrorPage(ServerConfig* server_config);

  void ParseLocation(ServerConfig* server_config);
  void ParseMethods(Location* location);
  void ParseRoot(Location* location);
  void ParseAutoIndex(Location* location);
  void ParseIndex(Location* location);
  void ParseUploadPath(Location* location);
  void ParseRedirect(Location* location);
  void ParseCgi(Location* location);
  template <typename T, typename Setter>
  void ParseSimpleDirective(T* obj, Setter setter,
                            const std::string& error_msg);

  const std::vector<ServerConfig>& GetServerConfigs() const {
    return server_configs_;
  }

  bool IsSafeIndexFilename(const std::string& filename) const;
};

template <typename T, typename Setter>
void ConfigParser::ParseSimpleDirective(T* obj, Setter setter,
                                        const std::string& error_msg) {
  std::string token = Tokenize(content);
  if (token.empty()) {
    throw std::runtime_error("Syntax error: expected " + error_msg);
  }
  (obj->*setter)(token);
  ConsumeExpectedSemicolon(error_msg);
}

#endif  // CONFIG_PARSER_HPP_
