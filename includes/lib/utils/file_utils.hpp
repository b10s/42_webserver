#ifndef FILE_UTILS_HPP_
#define FILE_UTILS_HPP_
#include <sys/stat.h>

#include <string>

namespace lib {
namespace utils {

bool IsDirectory(const std::string& path);

std::string ReadFile(const std::string& filename);

}  // namespace utils
}  // namespace lib

#endif  // FILE_UTILS_HPP_
