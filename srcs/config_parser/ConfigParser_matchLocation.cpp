#include "ConfigParser.hpp"
#include "lib/utils/string_utils.hpp"

const Location& ServerConfig::MatchLocation(const std::string& uri) const {
  const Location* longest_match = nullptr;
  size_t longest_length = 0;

  for (std::vector<Location>::const_iterator it = locations_.begin();
       it != locations_.end(); ++it) {
    const std::string& loc_name = it->GetName();
    if (lib::utils::StartsWith(uri, loc_name)) {
      size_t loc_length = loc_name.length();
      if (loc_length > longest_length) {
        longest_length = loc_length;
        longest_match = &(*it);
      }
    }
  }
  if (longest_match == nullptr) {
    throw std::runtime_error("No matching location found for URI: " + uri);
  }
  return *longest_match;
}
