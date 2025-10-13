#include "config_parser.hpp"

// Parses the entire configuration file and builds the server configuration objects.
ConfigParser::ConfigParser(const std::string &filename) : currentPos_(0) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    struct stat s;
    if (stat(filename.c_str(), &s) == 0) {
      if (s.st_mode & S_IFDIR) {
        throw std::runtime_error(filename + " is a directory");
      }
    } else {
      throw std::runtime_error("Failed to get file status: " + filename);
    }

    this->content_ = std::string(
      std::istreambuf_iterator<char>(file),
      std::istreambuf_iterator<char>()
    );
    this->parse();
}