#ifndef MIME_TYPE_HPP_
#define MIME_TYPE_HPP_

#include <map>
#include <string>
#include <cctype>

namespace lib {
namespace http {
    std::string DetectMimeTypeFromPath(const std::string& file_path);
}  // namespace http
}  // namespace lib

#endif  // MIME_TYPE_HPP_
