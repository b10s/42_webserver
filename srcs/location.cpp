#include "ConfigParser.hpp"

Location::Location()
    : methods_(),
      name_("/"),
      root_("./"),
      autoindex_(false),
      indexFiles_(),
      extensions_(),
      uploadPath_(),
      redirect_(),
      cgiPath_() {
}

// Remove this function from location.cpp and implement it in the correct class
// header and source files, for example:

// In config_parser.hpp (or the appropriate header file):
// class ConfigParser {
// public:
//     const std::vector<Location>& getLocations() const;
// private:
//     std::vector<Location> locations_;
// };

// In config_parser.cpp (or the appropriate source file):
// const std::vector<Location>& ConfigParser::getLocations() const {
//     return locations_;
// }
