#include <iostream>
#include <string>

#include "Webserv.hpp"

int main(int argc, char* argv[]) {
  try {
    std::string config_file =
        (argc > 1) ? argv[1] : "demo/conf/webserv_eval.conf";

    std::cout << "Loading configuration from: " << config_file << std::endl;

    Webserv webserver(config_file);
    webserver.Run();

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
