#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "ConfigParser.hpp"
#include "ServerConfig.hpp"
#include "Webserv.hpp"
#include "lib/utils/Bzero.hpp"
#include "Epoll.hpp"

int main(int argc, char* argv[]) {
  try {
    std::string config_file =
        (argc > 1) ? argv[1] : "sample_config/server_multiple_port.conf";

    std::cout << "Loading configuration from: " << config_file << std::endl;

    // Initialize webserver
    Webserv webserver(config_file);
    webserver.Run();

    // Run test
    // webserver.TestConfiguration();

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
