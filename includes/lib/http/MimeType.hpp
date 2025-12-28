#ifndef MIME_TYPE_HPP_
#define MIME_TYPE_HPP_

#include <cctype>
#include <map>
#include <string>

namespace lib {
namespace http {
std::string DetectMimeTypeFromPath(const std::string& file_path);
}  // namespace http
}  // namespace lib

#endif  // MIME_TYPE_HPP_
