#include "ConfigParser.hpp"
#include "lib/utils/string_utils.hpp"

// treat "/path////" as "/path"
// NOTE(routing): We intentionally do NOT normalize internal "//" in the URI path.
// Routing normalization only absorbs trailing-slash variations ("/img" vs "/img/").
// Any non-trivial normalization or dangerous patterns (e.g. "//", "..", percent-decoding)
// are handled later in the security/path-validation phase.
static std::string TrimTrailingSlashExceptRoot(const std::string& s) {
  if (s.size() <= 1) return s;              // "/"
  size_t end = s.size();
  while (end > 1 && s[end - 1] == '/') {
    --end;
  }
  return s.substr(0, end);
}

// URI and prefix are both assumed to be trimmed of trailing slashes (except for root "/")
bool ServerConfig::IsPathPrefix(const std::string& uri_key,
                                const std::string& loc_key) const {
  if (loc_key == "/") return true;
  if (uri_key.size() < loc_key.size()) return false;
  if (uri_key.compare(0, loc_key.size(), loc_key) != 0) return false;
  if (uri_key.size() == loc_key.size()) return true;
  if (uri_key[loc_key.size()] == '/') return true;
  return false;
}

// URI is the full request URI (e.g., "/images/logo.png")
const Location& ServerConfig::FindLocationForUri(const std::string& uri) const {
  const Location* longest_match = NULL;
  size_t longest_length = 0;
  const std::string uri_key = TrimTrailingSlashExceptRoot(uri);

  for (std::vector<Location>::const_iterator it = locations_.begin();
       it != locations_.end(); ++it) {
    const std::string& loc_key = TrimTrailingSlashExceptRoot(it->GetName());
    if (ServerConfig::IsPathPrefix(uri_key, loc_key)) {
      size_t loc_length = loc_key.length();
      if (loc_length > longest_length) {
        longest_length = loc_length;
        longest_match = &(*it);
      }
    }
  }
  if (longest_match == NULL) {
    throw std::runtime_error("No matching location found for URI: " + uri_key); // TODO: return HTTP 404?
  }
  return *longest_match;
}
