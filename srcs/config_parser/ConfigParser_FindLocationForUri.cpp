#include "ConfigParser.hpp"
#include "lib/utils/string_utils.hpp"
#include "LocationMatch.hpp"

// treat "/path////" as "/path"
// NOTE(routing): We intentionally do NOT normalize internal "//" in the URI
// path. Routing normalization only absorbs trailing-slash variations ("/img" vs
// "/img/"). Any non-trivial normalization or dangerous patterns (e.g. "//",
// "..", percent-decoding) are handled later in the security/path-validation
// phase.
static std::string TrimTrailingSlashExceptRoot(const std::string& s) {
  if (s.size() <= 1) return s;  // "/"
  size_t end = s.size();
  while (end > 1 && s[end - 1] == '/') {
    --end;
  }
  return s.substr(0, end);
}

// URI and prefix are both assumed to be trimmed of trailing slashes (except for
// root "/")
bool ServerConfig::IsPathPrefix(const std::string& uri_key,
                                const std::string& loc_key) const {
  if (loc_key == "/") return true; // root matches all
  if (uri_key.size() < loc_key.size()) return false;
  if (uri_key.compare(0, loc_key.size(), loc_key) != 0) return false;
  if (uri_key.size() == loc_key.size()) return true;
  if (uri_key[loc_key.size()] == '/') return true;
  return false;
}

// URI is the full request URI (e.g., "/images/logo.png")
// Trailing '/' can be present or absent so needs to be normalized
// *_key means trimmed of trailing slashes (except for root "/")
LocationMatch ServerConfig::FindLocationForUri(const std::string& uri) const {
  LocationMatch best;
  best.loc = NULL; // pointer to best matching location
  size_t best_len = 0;
  const std::string uri_key = TrimTrailingSlashExceptRoot(uri);

  for (std::vector<Location>::const_iterator it = locations_.begin();
       it != locations_.end(); ++it) {
    const std::string loc_key = TrimTrailingSlashExceptRoot(it->GetName());
    if (IsPathPrefix(uri_key, loc_key)) {
      size_t len = loc_key.size();
      if (len > best_len) {
        best_len = len;
        best.loc = &(*it); // Location pointer
      }
    }
  }
  if (!best.loc) {
    throw std::runtime_error("No matching location found for URI: " +
                             uri_key);  // TODO: return HTTP 404?
  }
  const std::string best_key = TrimTrailingSlashExceptRoot(best.loc->GetName());
  // build remainder (make sure remainder always starts with '/')
  if (uri_key.size() == best_key.size()) { // exact match
    best.remainder = "/";
  } else {
    best.remainder = uri_key.substr(best_key.size());
    if (best.remainder.empty()) best.remainder = "/"; // should not happen
    if (best.remainder[0] != '/') best.remainder = "/" + best.remainder;
  }
  return best;
}

LocationMatch ServerConfig::ResolveLocationForUri(
    const std::string& uri) const {
  LocationMatch result;
  result.loc = &FindLocationForUri(uri);
}
