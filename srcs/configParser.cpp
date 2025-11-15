#include "ConfigParser.hpp"

ConfigParser::ConfigParser() : current_pos_(0), server_configs_(), content_("") {
}

ConfigParser::ConfigParser(const std::string& text)
    : current_pos_(0), server_configs_(), content_(text) {
}

ConfigParser::~ConfigParser() {
}

void ConfigParser::loadFile(const std::string& filename) {
  struct stat s;
  if (stat(filename.c_str(), &s) != 0) {
    throw std::runtime_error("File does not exist: " + filename);
  }
  if (S_ISDIR(s.st_mode)) {
    throw std::runtime_error(filename + " is a directory");
  }
  std::ifstream file(filename.c_str());
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }
  content_.assign(std::istreambuf_iterator<char>(file),
                  std::istreambuf_iterator<char>());
  file.close();
}
