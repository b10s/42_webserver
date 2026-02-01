#include "lib/utils/file_utils.hpp"

namespace lib {
namespace utils {

bool IsDirectory(const std::string& path) {
  struct stat st;
  if (stat(path.c_str(), &st) != 0) return false;
  return S_ISDIR(st.st_mode);
}

/*
map errno -> HTTP status
- ENOENT(Error No ENTry/Entity) -> resource not found (404)
- ENOTDIR(Error Not a DIRectory) -> resource not found (404)
- EACCES          -> permission denied (403)
- others          -> internal server error (500)
*/
lib::http::Status MapErrnoToHttpStatus(int e) {
  switch (e) {
    case ENOENT:
    case ENOTDIR:
      return lib::http::kNotFound;  // 404
    case EACCES:
    case EPERM:
      return lib::http::kForbidden;  // 403
    case EISDIR:
      return lib::http::kBadRequest;  // 400
    default:
      return lib::http::kInternalServerError;  // 500
  }
}

struct stat StatOrThrow(const std::string& path) {
  struct stat st;
  if (stat(path.c_str(), &st) != 0) {
    int saved_errno = errno;
    throw lib::exception::ResponseStatusException(
        MapErrnoToHttpStatus(saved_errno));
  }
  return st;
}

void EnsureAccessOrThrow(const std::string& path, int mode) {
  if (access(path.c_str(), mode) != 0) {
    int saved_errno = errno;
    throw lib::exception::ResponseStatusException(
        MapErrnoToHttpStatus(saved_errno));
  }
}

// in this project, reject non-regular files (directories, symlinks, devices,
// etc) as forbidden (403) for simplicity
void EnsureRegularFileOrThrowForbidden(const struct stat& st) {
  if (!S_ISREG(st.st_mode)) {
    throw lib::exception::ResponseStatusException(http::kForbidden);
  }
}

// read entire file content into a string
std::string ReadFileToStringOrThrow(const std::string& filename) {
  struct stat buffer = StatOrThrow(filename);
  EnsureRegularFileOrThrowForbidden(buffer);
  EnsureAccessOrThrow(filename, R_OK);
  std::ifstream file(filename.c_str());
  if (!file.is_open()) {  // errno might be unreliable here
    int saved_errno = errno;
    throw lib::exception::ResponseStatusException(
        MapErrnoToHttpStatus(saved_errno));
  }
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();
  return content;
}

// static file GET
void CheckReadableRegularFileOrThrow(const std::string& path) {
  struct stat buffer = StatOrThrow(path);
  EnsureRegularFileOrThrowForbidden(buffer);
  EnsureAccessOrThrow(path, R_OK);
}

// CGI script execution (GET/POST)
void CheckExecutableCgiScriptOrThrow(const std::string& path) {
  struct stat buffer = StatOrThrow(path);
  EnsureRegularFileOrThrowForbidden(buffer);
  EnsureAccessOrThrow(path, X_OK);
}

/*
parent dir must be executable and writable when deleting a file
dirname destroys its argument, so copy it first
and its argument must be writable buffer
example:
  std::string path = "/a/b/c.txt";
  char* path_copy = &path[0];
  // after this call, path's buffer may look like "/a/b\0c.txt" in memory,
  // and dirname(path_copy) returns "/a/b"
*/
void CheckDeletableRegularFileOrThrow(const std::string& path) {
  struct stat buffer = StatOrThrow(path);
  EnsureRegularFileOrThrowForbidden(buffer);
  std::string path_copy = path;
  std::string dir_name = dirname(&path_copy[0]);
  if (stat(dir_name.c_str(), &buffer) != 0) {
    int saved_errno = errno;
    throw lib::exception::ResponseStatusException(
        MapErrnoToHttpStatus(saved_errno));
  }
  if (access(dir_name.c_str(), W_OK | X_OK) != 0) {
    int saved_errno = errno;
    throw lib::exception::ResponseStatusException(
        MapErrnoToHttpStatus(saved_errno));
  }
}

}  // namespace utils
}  // namespace lib
