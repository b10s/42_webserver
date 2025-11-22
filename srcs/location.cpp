#include "ConfigParser.hpp"

Location::Location()
    : methods_(),
      name_("/"),
      root_("./"),
      autoindex_(false),
      index_files_(),
      extensions_(),
      upload_path_(),
      redirect_(),
      cgi_path_(),
      has_root_(false),
      has_autoindex_(false),
      has_extensions_(false),
      has_upload_path_(false),
      has_redirect_(false),
      has_cgi_path_(false) {
}

// Remove this function from location.cpp and implement it in the correct class
// header and source files, for example:

// In config_parser.hpp (or the appropriate header file):
// class ConfigParser {
// public:
//     const std::vector<Location>& GetLocations() const;
// private:
//     std::vector<Location> locations_;
// };

// In config_parser.cpp (or the appropriate source file):
// const std::vector<Location>& ConfigParser::GetLocations() const {
//     return locations_;
// }
