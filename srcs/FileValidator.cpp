#include "FileValidator.hpp"

#include "lib/exception/ResponseStatusException.hpp"
#include "lib/http/CharValidation.hpp"

/*
仕様
percent-decode しない
NUL/制御文字拒否 : lib/http/CharValidation.hpp の IsValidHeaderChar() を流用

ContainsDotDotSegment(path):
- 1回でも出たら即 reject（400/404）
- (移動するならstackやdequeで管理したほうがよいかも？いったん保留)
- reject:`/../a`  `/a/../b` `/a/..` / `..`
- rejectしない例（ファイル名）:  `/a..b` / `/a/..b` / `/a/b..`

I don't think we need to resolve symlinks in this project
*/

// Check for control characters (allow spaces)
bool FileValidator::ContainsUnsafeChars(const std::string& path) {
  for (size_t i = 0; i < path.size(); ++i) {
    char c = path[i];
    if (!lib::http::IsValidHeaderChar(c)) {
      return true;
    }
  }
  return false;
}

// split path into segments by '/'
// empty segments are ignored
std::vector<std::string> FileValidator::SplitPathSegments(
    const std::string& path) {
  std::vector<std::string> segments;
  size_t start = 0;
  size_t end = 0;
  while ((end = path.find('/', start)) != std::string::npos) {
    if (end != start) {  // avoid empty segment
      segments.push_back(path.substr(start, end - start));
    }
    start = end + 1;
  }
  if (start < path.size()) {  // last segment
    segments.push_back(path.substr(start));
  }
  return segments;
}

// Normalize path using a stack of segments
// ".." segments are rejected when they try to escape the root
// - absolute path: "/../x" tries to escape root -> reject
// - relative path: "../x" cannot be normalized safely without a base -> reject
std::string FileValidator::NormalizePathBySegments(const std::string& path) {
  const bool has_leading_slash = !path.empty() && path[0] == '/';
  const bool has_trailing_slash = !path.empty() && path[path.size() - 1] == '/';

  std::vector<std::string> segments = SplitPathSegments(path);
  std::vector<std::string> stack;
  stack.reserve(segments.size());
  for (size_t i = 0; i < segments.size(); ++i) {
    const std::string& segment = segments[i];
    if (segment == ".") {
      continue;
    } else if (segment == "..") {
      if (stack.empty()) {
        throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
      }
      stack.pop_back();
      continue;
    }
    stack.push_back(segment);
  }

  std::string rebuilt_path;
  if (has_leading_slash) rebuilt_path += '/';
  for (size_t i = 0; i < stack.size(); ++i) {
    if (i > 0) rebuilt_path += '/';
    rebuilt_path += stack[i];
  }
  if (has_trailing_slash && !rebuilt_path.empty() &&
      rebuilt_path[rebuilt_path.size() - 1] != '/') {
    rebuilt_path += '/';
  }
  if (has_leading_slash && rebuilt_path.empty()) return "/";
  return rebuilt_path;
}

bool FileValidator::IsPathUnderDocumentRoot(const std::string& path,
                                            const std::string& document_root) {
  if (document_root.empty()) return false;
  if (document_root == "/") return true;  // root matches all
  if (path.size() < document_root.size()) return false;
  if (path.compare(0, document_root.size(), document_root) != 0) return false;
  if (path.size() == document_root.size()) return true;  // exact match
  return path[document_root.size()] == '/';              // boundary check
}

// path always starts with "/" (absolute path)
std::string FileValidator::ValidateAndNormalizePath(
    const std::string& path, const std::string& document_root) {
  if (ContainsUnsafeChars(path)) {
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  }
  std::string normalized_path = NormalizePathBySegments(path);
  if (!IsPathUnderDocumentRoot(normalized_path, document_root)) {
    throw lib::exception::ResponseStatusException(lib::http::kNotFound);
  }
  return normalized_path;
}
