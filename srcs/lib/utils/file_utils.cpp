#include "lib/utils/file_utils.hpp"

#include <sys/stat.h>

#include <fstream>
#include <stdexcept>

namespace lib {
namespace utils {

bool IsDirectory(const std::string& path) {
  struct stat st;
  if (stat(path.c_str(), &st) != 0) return false;
  return S_ISDIR(st.st_mode);
}

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

// map errno -> HTTP status
lib::http::Status MapErrnoToStatus(int e) {
  if (e == ENOENT || e == ENOTDIR) return lib::http::kNotFound;  // 404
  if (e == EACCES) return lib::http::kForbidden;                 // 403
  return lib::http::kInternalServerError;
}

void EnsureReadableRegularFileOrThrow(const std::string& path) {
  struct stat st;
  if (stat(path.c_str(), &st) != 0) {
    throw lib::exception::ResponseStatusException(MapErrnoToStatus(errno));
  }
  if (!S_ISREG(st.st_mode)) {  // directory or device etc. are not valid for
                               // direct GET
    throw lib::exception::ResponseStatusException(lib::http::kForbidden);
  }
  if (access(path.c_str(), R_OK) != 0) {
    throw lib::exception::ResponseStatusException(MapErrnoToStatus(errno));
  }
}

void EnsureExecutableRegularFileOrThrow(const std::string& path) {
  struct stat st;
  if (stat(path.c_str(), &st) != 0) {
    throw lib::exception::ResponseStatusException(MapErrnoToStatus(errno));
  }
  if (!S_ISREG(st.st_mode)) {
    throw lib::exception::ResponseStatusException(lib::http::kForbidden);
  }
  if (access(path.c_str(), X_OK) != 0) {
    throw lib::exception::ResponseStatusException(MapErrnoToStatus(errno));
  }
}

}  // namespace utils
}  // namespace lib
