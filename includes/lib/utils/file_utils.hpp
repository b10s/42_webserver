#ifndef FILE_UTILS_HPP_
#define FILE_UTILS_HPP_
#include <sys/stat.h>
#include <string>
#include <cerrno>
#include "lib/exception/ResponseStatusException.hpp"
#include "lib/http/Status.hpp"
#include <unistd.h>

namespace lib {
namespace utils {

// OS file system utilities
bool IsDirectory(const std::string& path);
std::string ReadFile(const std::string& filename);

// file existence and permission checks
// if no access, throw 403(kForbidden) or 404(kNotFound)
lib::http::Status MapErrnoToStatus(int e);
void EnsureReadableRegularFileOrThrow(const std::string& path);
void EnsureExecutableRegularFileOrThrow(const std::string& path);

}  // namespace utils
}  // namespace lib

#endif  // FILE_UTILS_HPP_
