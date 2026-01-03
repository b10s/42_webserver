#ifndef FILE_UTILS_HPP_
#define FILE_UTILS_HPP_
#include <sys/stat.h>
#include <unistd.h> // for access()
#include <cerrno>
#include <string>
#include <libgen.h>  // for dirname
#include <fstream>

#include "lib/exception/ResponseStatusException.hpp"
#include "lib/http/Status.hpp"

namespace lib {
namespace utils {

// TODO: refactoring ResolveFilesystemPath may obsolete this function
bool IsDirectory(const std::string& path);

std::string ReadStaticFileOrThrow(const std::string& filename);

// file existence and permission checks
// if no access, throw 403(kForbidden) or 404(kNotFound)
lib::http::Status MapErrnoToStatus(int e);
void EnsureExecutableRegularFileOrThrow(const std::string& path);

}  // namespace utils
}  // namespace lib

#endif  // FILE_UTILS_HPP_
