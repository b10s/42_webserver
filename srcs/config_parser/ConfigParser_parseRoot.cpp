#include <limits.h>  // PATH_MAX
#include <unistd.h>  // getcwd

#include "ConfigParser.hpp"
#include "FileValidator.hpp"
#include "lib/http/CharValidation.hpp"

namespace {
// Anonymous namespace for internal helper functions

bool IsAbsolutePath(const std::string& path) {
  return !path.empty() && path[0] == '/';
}

std::string JoinAndNormalizePath(const std::string& base,
                                 const std::string& relative) {
  if (base.empty() || base == ".") {
    return relative;
  }
  std::string joined = base + "/" + relative;
  return FileValidator::NormalizePathBySegments(joined);
}

std::string GetCwdOrThrow() {
  char cwd[PATH_MAX];
  if (getcwd(cwd, PATH_MAX) == NULL) {
    throw std::runtime_error("Failed to get current working directory");
  }
  return std::string(cwd);
}
}  // namespace

std::string ConfigParser::ResolveRootPath(const std::string& token) const {
  if (IsAbsolutePath(token)) return token;
  std::string current_dir = GetCwdOrThrow();
  std::string base_dir = JoinAndNormalizePath(current_dir, token);
  return base_dir;
}

void ConfigParser::ParseRoot(Location* location) {
  std::string token = Tokenize(content);
  if (token.empty()) {
    throw std::runtime_error(
        "Syntax error: expected root path but got empty token");
  }
  std::string resolved = ResolveRootPath(token);
  RequireAbsoluteSafePathOrThrow(resolved, "Root path");
  location->SetRoot(resolved);
  ConsumeExpectedSemicolon("root path");
}
