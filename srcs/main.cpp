#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "config.hpp"

int main() {
  // config_sample_files/sample.conf を読み込む例
  std::ifstream file("sample_config/server_localhost.conf");
  if (!file) {
    std::cerr << "Failed to open config file." << std::endl;
    return 1;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string content = buffer.str();

  std::string token;
  Config config("sample_config/server_localhost.conf");
  while (!(token = config.tokenize(content)).empty()) {
    std::cout << token << std::endl;
  }

  return 0;
}
