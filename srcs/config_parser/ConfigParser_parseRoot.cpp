#include <unistd.h>  // getcwd
#include <limits.h> // PATH_MAX

#include "ConfigParser.hpp"
#include "lib/http/CharValidation.hpp"

static bool IsAbsolutePath(const std::string& path) {
  return !path.empty() && path[0] == '/';
}

static std::string JoinPath(const std::string& base, const std::string& rel) {
  if (base.empty() || base == ".") return rel;
  if (!base.empty() && base[base.size() - 1] == '/') return base + rel;
  return base + "/" + rel;
}

std::string ConfigParser::ResolvePathRelativeToConfig(
    const std::string& token) const {
  if (IsAbsolutePath(token)) return token;
  char cwd[PATH_MAX];
  if (getcwd(cwd, PATH_MAX) == NULL) {
    throw std::runtime_error("Failed to get current working directory");
  }
  std::string current_dir(cwd);
  return JoinPath(current_dir, token);
}

void ConfigParser::ParseRoot(Location* location) {
  std::string token = Tokenize(content);
  if (token.empty()) {
    throw std::runtime_error(
        "Syntax error: expected root path but got empty token");
  }
  std::string resolved_path = ResolvePathRelativeToConfig(token);
#ifndef WEBSERV_DEBUG
  std::cerr << "Resolved root path: " << resolved_path << std::endl;
#endif
  RequireAbsoluteSafePathOrThrow(resolved_path, "Root path");
  location->SetRoot(resolved_path);
  ConsumeExpectedSemicolon("root path");
}
