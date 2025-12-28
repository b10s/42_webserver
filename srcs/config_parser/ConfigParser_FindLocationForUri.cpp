#include "ConfigParser.hpp"
#include "lib/utils/string_utils.hpp"

bool ServerConfig::IsPathPrefix(const std::string& uri,
                                const std::string& prefix) const {
  if (prefix == "/") return true;
  if (uri.size() < prefix.size()) return false;
  if (uri.compare(0, prefix.size(), prefix) != 0) return false;
  if (uri.size() == prefix.size()) return true;
  if (uri[prefix.size()] == '/') return true;
  return false;
}

// URI is the full request URI (e.g., "/images/logo.png")
const Location& ServerConfig::FindLocationForUri(const std::string& uri) const {
  const Location* longest_match = NULL;
  size_t longest_length = 0;

  for (std::vector<Location>::const_iterator it = locations_.begin();
       it != locations_.end(); ++it) {
    const std::string& loc_name = it->GetName();
    if (ServerConfig::IsPathPrefix(uri, loc_name)) {
      size_t loc_length = loc_name.length();
      if (loc_length > longest_length) {
        longest_length = loc_length;
        longest_match = &(*it);
      }
    }
  }
  if (longest_match == NULL) {
    throw std::runtime_error("No matching location found for URI: " + uri);
  }
  return *longest_match;
}
