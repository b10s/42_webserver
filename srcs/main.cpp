#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "config.hpp"

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
//     while (!(token = config.tokenize(content)).empty()) {
//         std::cout << token << std::endl;
//     }

//     return 0;
// }


int main() {
    try {
        Config config("sample_config/server_no_location.conf");
        const std::vector<ServerConfig>& servers = config.getServerConfigs();
        for (size_t i = 0; i < servers.size(); ++i) {
            std::cout << "Server " << i << ":\n";
            std::cout << "  Host: " << servers[i].getHost() << "\n";
            std::cout << "  Port: " << servers[i].getPort() << "\n";
            std::cout << "  Server Name: " << servers[i].getServerName() << "\n";
            std::cout << "  Max Body Size: " << servers[i].getMaxBodySize() << "\n";
            std::cout << "  Error Pages:\n" << servers[i].getErrorPagesString() << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
