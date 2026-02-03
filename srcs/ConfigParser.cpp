#include "ConfigParser.hpp"

#include "lib/utils/file_utils.hpp"

ConfigParser::ConfigParser() : current_pos_(0), server_configs_(), content("") {
}

ConfigParser::ConfigParser(const std::string& text)
    : current_pos_(0), server_configs_(), content(text) {
}

ConfigParser::~ConfigParser() {
}

static std::string RealPathOrThrow(const std::string& path) {
  char real_path_buf[PATH_MAX];
  if (realpath(path.c_str(), real_path_buf) == NULL) {
    throw std::runtime_error("Failed to resolve real path: " + path);
  }
  return std::string(real_path_buf);
}

static std::string DirnameOf(const std::string& path) {
  std::string::size_type pos = path.find_last_of('/');
  if (pos == std::string::npos) return "."; // current directory
  if (pos == 0) return "/"; // root directory
  return path.substr(0, pos); // for example, /a/b/c.txt -> /a/b
}

void ConfigParser::LoadFileOrThrowRuntime(const std::string& filename) {
  struct stat s;
  if (stat(filename.c_str(), &s) != 0) {
    throw std::runtime_error("File does not exist: " + filename);
  }
  if (S_ISDIR(s.st_mode)) {
    throw std::runtime_error(filename + " is a directory");
  }
  std::string real_path = RealPathOrThrow(filename);
  config_dir_ = DirnameOf(real_path);
  std::ifstream file(filename.c_str());
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }
  content = std::string((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
  file.close();
}

void ConfigParser::RequireAbsoluteSafePathOrThrow(const std::string& path,
                                                  const std::string& label) {
  if (path.empty() || path[0] != '/') {
    throw std::runtime_error(label + " must be an absolute path: " + path);
  }
  for (size_t i = 0; i < path.size(); ++i) {
    if (!lib::http::IsValidHeaderChar(path[i])) {
      throw std::runtime_error(label + " contains invalid character: " + path);
    }
  }
}
