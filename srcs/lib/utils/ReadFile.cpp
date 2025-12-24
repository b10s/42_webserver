#include "lib/utils/ReadFile.hpp"

#include <sys/stat.h>

#include <fstream>
#include <stdexcept>

namespace lib {
namespace utils {

std::string ReadFile(const std::string& filename) {
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
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();
  return content;
}

}  // namespace utils
}  // namespace lib
