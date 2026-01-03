#ifndef FILE_UTILS_HPP_
#define FILE_UTILS_HPP_
#include <libgen.h>  // for dirname
#include <sys/stat.h>
#include <unistd.h>  // for access()

#include <cerrno>
#include <fstream>
#include <string>

#include "lib/exception/ResponseStatusException.hpp"
#include "lib/http/Status.hpp"

namespace lib {
namespace utils {

bool IsDirectory(const std::string& path);

// file existence and permission checks
// if no access, throw 403(kForbidden) or 404(kNotFound)
lib::http::Status MapErrnoToHttpStatus(int e);

struct stat StatOrThrow(const std::string& path);
void EnsureAccessOrThrow(const std::string& path, int mode);
void EnsureRegularFileOrThrowForbidden(const struct stat& st);

// DELETE method permission check
void CheckDeletableRegularFileOrThrow(const std::string& path);

// read entire file content into a string (load config file, etc)
std::string ReadFileToStringOrThrow(const std::string& filename);

// static file GET
void CheckReadableRegularFileOrThrow(const std::string& path);
// CGI
void CheckExecutableCgiScriptOrThrow(const std::string& path);

}  // namespace utils
}  // namespace lib

#endif  // FILE_UTILS_HPP_
