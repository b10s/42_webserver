#include "lib/utils/file_utils.hpp"
#include "lib/exception/ResponseStatusException.hpp"
#include "lib/http/Status.hpp"

#include <sys/stat.h>

#include <fstream>
#include <stdexcept>
#include <cerrno>
#include <unistd.h> // for access()
#include <libgen.h> // for dirname

namespace lib {
namespace utils {

/*
map errno -> HTTP status
- ENOENT(Error No ENTry/Entity) -> resource not found (404)
- ENOTDIR(Error Not a DIRectory) -> resource not found (404)
- EACCES          -> permission denied (403)
- others          -> internal server error (500)
*/
lib::http::Status MapErrnoToStatus(int e) {
  if (e == ENOENT || e == ENOTDIR) return lib::http::kNotFound;  // 404
  if (e == EACCES) return lib::http::kForbidden;                 // 403
  return lib::http::kInternalServerError;
}

// index file appending needed 
// when the path is a directory and method is GET
bool IsDirectory(const std::string& path) {
  struct stat st;
  if (stat(path.c_str(), &st) != 0) return false;
  return S_ISDIR(st.st_mode);
}

struct stat StatOrThrow(const std::string& path) {
  struct stat st;
  if (stat(path.c_str(), &st) != 0) {
    int saved_errno = errno;
    throw lib::exception::ResponseStatusException(MapErrnoToStatus(saved_errno));
  }
  return st;
}

void EnsureAccessOrThrow(const std::string& path, int mode) {
  if (access(path.c_str(), mode) != 0) {
    int saved_errno = errno;
    throw lib::exception::ResponseStatusException(MapErrnoToStatus(saved_errno));
  }
}

// in this project, reject non-regular files (directories, symlinks, devices, etc)
// as forbidden (403) for simplicity
void EnsureRegularFileOrThrowForbidden(const struct stat& st) {
  if (!S_ISREG(st.st_mode)) {
    throw lib::exception::ResponseStatusException(http::kForbidden);
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
  struct stat buffer = StatOrThrow(path);
  EnsureRegularFileOrThrowForbidden(buffer);
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

// read entire file content into a string
std::string ReadStaticFileOrThrow(const std::string& filename) {
  struct stat buffer = StatOrThrow(filename);
  EnsureRegularFileOrThrowForbidden(buffer);
  EnsureAccessOrThrow(filename, R_OK);
  std::ifstream file(filename.c_str());
  if (!file.is_open()) {
    int saved_errno = errno;
    throw lib::exception::ResponseStatusException(MapErrnoToStatus(saved_errno));
  }
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();
  return content;
}

// static file GET
inline void ValidateReadableStaticFile(const std::string& path) {
  struct stat buffer = StatOrThrow(path);
  EnsureRegularFileOrThrowForbidden(buffer);
  EnsureAccessOrThrow(path, R_OK);
}

// CGI script execution (GET/POST)
inline void ValidateExecutableCgiScript(const std::string& path) {
  struct stat buffer = StatOrThrow(path);
  EnsureRegularFileOrThrowForbidden(buffer);
  EnsureAccessOrThrow(path, X_OK);
}

}  // namespace utils
}  // namespace lib
