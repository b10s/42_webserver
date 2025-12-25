#include "lib/http/MimeType.hpp"
#include "lib/utils/string_utils.hpp"

namespace lib {
namespace http {

static std::string GetExtension(const std::string& file_path) {
    std::string path = file_path;
    std::string::size_type question = path.find('?');
    if (question != std::string::npos) {
        path = path.substr(0, question);
    }
    std::string::size_type last_dot = path.find_last_of('.');
    if (last_dot == std::string::npos) return "";
    return lib::utils::ToLowerAscii(path.substr(last_dot + 1));
}

std::string DetectMimeTypeFromPath(const std::string& file_path) {
    static std::map<std::string, std::string> mime_types;
    if (mime_types.empty()) {
        mime_types["html"] = "text/html";
        mime_types["htm"] = "text/html";
        mime_types["css"] = "text/css";
        mime_types["js"] = "application/javascript";
        mime_types["json"] = "application/json";
        mime_types["png"] = "image/png";
        mime_types["jpg"] = "image/jpeg";
        mime_types["jpeg"] = "image/jpeg";
        mime_types["gif"] = "image/gif";
        mime_types["txt"] = "text/plain";
        mime_types["pdf"] = "application/pdf";
        mime_types["xml"] = "application/xml";
        mime_types["svg"] = "image/svg+xml";
        mime_types["ico"] = "image/x-icon";
    }

    const std::string extension = GetExtension(file_path);
    std::map<std::string, std::string>::const_iterator it = mime_types.find(extension);
    if (it != mime_types.end()) return it->second;
    return "application/octet-stream";  // Default MIME type
}

}  // namespace http
}  // namespace lib
