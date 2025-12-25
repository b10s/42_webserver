#include <sys/stat.h>

// uri ends with '/' or is a directory
static bool IsDirectory(const std::string& path) {
  struct stat st;
  if (stat(path.c_str(), &st) != 0) return false;
  return S_ISDIR(st.st_mode);
}
