#include "lib/http/MimeType.hpp"

#include "lib/utils/string_utils.hpp"

namespace lib {
namespace http {

static std::string GetExtension(const std::string& file_path) {
  std::string path = file_path;
  std::string::size_type last_dot = path.find_last_of('.');
  if (last_dot == std::string::npos) return "";
  return lib::utils::ToLowerAscii(path.substr(last_dot + 1));
}

// map is not thread safe, so we use a simple function here
std::string DetectMimeTypeFromPath(const std::string& file_path) {
  const std::string extension = GetExtension(file_path);
  if (extension == "html" || extension == "htm") return "text/html";
  if (extension == "css") return "text/css";
  if (extension == "js") return "application/javascript";
  if (extension == "json") return "application/json";
  if (extension == "png") return "image/png";
  if (extension == "jpg" || extension == "jpeg") return "image/jpeg";
  if (extension == "gif") return "image/gif";
  if (extension == "txt") return "text/plain";
  if (extension == "pdf") return "application/pdf";
  if (extension == "xml") return "application/xml";
  return "application/octet-stream";  // Default MIME type
}

}  // namespace http
}  // namespace lib
