#include "lib/utils/file_utils.hpp"

#include <sys/stat.h>

#include <fstream>
#include <stdexcept>
#include <libgen.h> // for dirname

namespace lib {
namespace utils {

bool IsDirectory(const std::string& path) {
  struct stat st;
  if (stat(path.c_str(), &st) != 0) return false;
  return S_ISDIR(st.st_mode);
}

struct stat StatOrThrow(const std::string& path) {
  struct stat st;
  if (stat(path.c_str(), &st) != 0) {
    int saved_errno = errno;
    throw ResponseStatusException(MapErrnoToStatus(saved_errno));
  }
  return st;
}

void EnsureAccessOrThrow(const std::string& path, int mode) {
  if (access(path.c_str(), mode) != 0) {
    int saved_errno = errno;
    throw ResponseStatusException(MapErrnoToStatus(saved_errno));
  }
}

void EnsureRegularFileOrThrow(const struct stat& st) {
  if (!S_ISREG(st.st_mode)) {
    throw ResponseStatusException(kForbidden);
  }
}

bool IsExecutable(const std::string& path) {
  if (access(path.c_str(), X_OK) != 0) {
    return false;
  }
  return true;
}

// map errno -> HTTP status
// ENOENT(Error No ENTry/Entity) -> resource not found (404)
// ENOTDIR(Error Not a DIRectory) -> resource not found (404)
// EACCES          -> permission denied (403)
// others          -> internal server error (500)
lib::http::Status MapErrnoToStatus(int e) {
  if (e == ENOENT || e == ENOTDIR) return lib::http::kNotFound;  // 404
  if (e == EACCES) return lib::http::kForbidden;                 // 403
  return lib::http::kInternalServerError;
}

// S_ISREG: is regular file (not directory, not symlink, not device, etc)
std::string ReadFileOrThrow(const std::string& filename) {
  struct stat st;
  
  if (access(filename.c_str(), R_OK) != 0) { // not readable, throw 403/404
    int saved_errno = errno;
    throw lib::exception::ResponseStatusException(MapErrnoToStatus(saved_errno));
  }
  std::ifstream file(filename.c_str());
  if (!file.is_open()) { // should not happen?
    int saved_errno = errno;
    throw lib::exception::ResponseStatusException(MapErrnoToStatus(saved_errno));
  }
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();
  return content;
}

void EnsureExecutableRegularFileOrThrow(const std::string& path) {
  struct stat st;
  if (stat(path.c_str(), &st) != 0) {
    int saved_errno = errno;
    throw lib::exception::ResponseStatusException(MapErrnoToStatus(saved_errno));
  }
  if (!S_ISREG(st.st_mode)) {
    throw lib::exception::ResponseStatusException(lib::http::kForbidden);
  }
  if (access(path.c_str(), X_OK) != 0) {
    int saved_errno = errno;
    throw lib::exception::ResponseStatusException(MapErrnoToStatus(saved_errno));
  }
}

/*
parent dir must be executable and writable when deleting a file
dirname destroys its argument, so copy it first
and its argument must be writable buffer
example:
  std::string path = "/a/b/c.txt";
  char* path_copy = &path[0];
  dirname(path_copy) returns "a/b\0c.txt" (modifies path_copy)
*/
void EnsureDeletableRegularFileOrThrow(const std::string& path) {
  struct stat buffer;
  std::string path_copy = path;
  std::string dir_name = dirname(&path_copy[0]);
  if (stat(dir_name.c_str(), &buffer) != 0) {
    int saved_errno = errno;
    throw lib::exception::ResponseStatusException(MapErrnoToStatus(saved_errno));
  }
  if (access(dir_name.c_str(), W_OK | X_OK) != 0) {
    int saved_errno = errno;
    throw lib::exception::ResponseStatusException(MapErrnoToStatus(saved_errno));
  }
}

}  // namespace utils
}  // namespace lib
