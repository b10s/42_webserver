#include "ConfigParser.hpp"
#include "lib/utils/ReadFile.hpp"

ConfigParser::ConfigParser() : current_pos_(0), server_configs_(), content("") {
}

ConfigParser::ConfigParser(const std::string& text)
    : current_pos_(0), server_configs_(), content(text) {
}

ConfigParser::~ConfigParser() {
}

void ConfigParser::LoadFile(const std::string& filename) {
  content = lib::utils::ReadFile(filename);
}
