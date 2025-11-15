#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "ConfigParser.hpp"
#include "ServerConfig.hpp"

// int main() {
//     // config_sample_files/sample.conf を読み込む例
//     std::ifstream file("sample_config/server_localhost.conf");
//     if (!file) {
//         std::cerr << "Failed to open config file." << std::endl;
//         return 1;
//     }

//     std::stringstream buffer;
//     buffer << file.rdbuf();
//     std::string content = buffer.str();

//     std::string token;
//     Config config("sample_config/server_localhost.conf");
//     while (!(token = config.Tokenize(content)).empty()) {
//         std::cout << token << std::endl;
//     }

//     return 0;
// }

int main() {
  try {
    ConfigParser config;
    config.loadFile("sample_config/server_test_for_parse.conf");
    config.parse();
    const std::vector<ServerConfig>& servers = config.getServerConfigs();
    for (size_t i = 0; i < servers.size(); ++i) {
      std::cout << "Server " << i << ":\n";
      std::cout << "  Host: " << servers[i].getHost() << "\n";
      std::cout << "  Port: " << servers[i].getPort() << "\n";
      std::cout << "  Server Name: " << servers[i].getServerName() << "\n";
      std::cout << "  Max Body Size: " << servers[i].getMaxBodySize() << "\n";
      std::cout << "  Error Pages:\n"
                << servers[i].getErrorPagesString() << "\n";

      std::cout << "  Locations:\n";
      const std::vector<Location>& locations = servers[i].getLocations();
      for (size_t j = 0; j < locations.size(); ++j) {
        std::cout << "    Location " << j << ":\n";
        std::cout << "      Name: " << locations[j].GetName() << "\n";
        std::cout << "      Root: " << locations[j].GetRoot() << "\n";
        std::cout << "      Autoindex: "
                  << (locations[j].GetAutoIndex() ? "on" : "off") << "\n";
        std::cout << "      Index Files: ";
        const std::vector<std::string>& index_files =
            locations[j].GetIndexFiles();
        for (size_t k = 0; k < index_files.size(); ++k) {
          std::cout << index_files[k] << " ";
        }
        std::cout << "\n";
        std::cout << "      Extensions: " << locations[j].GetExtensions();
        std::cout << "\n";
        std::cout << "      Upload Path: " << locations[j].getUploadPath()
                  << "\n";
        std::cout << "      Redirect: " << locations[j].getRedirect() << "\n";
        std::cout << "      CGI Path: " << locations[j].getCgiPath() << "\n";
      }
      std::cout << "\n";
    }
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
