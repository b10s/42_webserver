#include "FileValidater.hpp"

#include <iostream>

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

// Check for spaces or control characters
bool FileValidater::ContainsUnsafeChars(const std::string& path) {
  for (size_t i = 0; i < path.size(); ++i) {
    char c = path[i];
    if (!lib::http::IsVisibleAscii(c)) {
      return true;
    }
  }
  return false;
}

// reject: "/a/../b", "/..", "/a/..", "../a", ".."
bool FileValidater::ContainsDotDotSegments(const std::string& path) {
  std::vector<std::string> segments = SplitPathSegments(path);
  for (size_t i = 0; i < segments.size(); ++i) {
    if (segments[i] == "..") return true;
  }
  return false;
}

// "/////" を "/" に変換
std::string FileValidater::NormalizeSlashes(const std::string& path) {
  std::string result;
  bool previous_slash = false;
  for (size_t i = 0; i < path.size(); ++i) {
    char c = path[i];
    if (c == '/') {
      if (!previous_slash) {
        result += c;
        previous_slash = true;
      }
    } else {
      result += c;
      previous_slash = false;
    }
  }
  return result;
}

// split path into segments by '/'
// もし..を今後移動に使うなら stack/deque にするかも
std::vector<std::string> FileValidater::SplitPathSegments(
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

std::string FileValidater::RemoveSingleDotSegments(const std::string& path) {
  std::string result;
  std::vector<std::string> segments = SplitPathSegments(path);
  for (size_t i = 0; i < segments.size(); ++i) {
    if (segments[i] == ".") continue;  // skip
    if (!result.empty()) result += '/';
    result += segments[i];
  }
  if (path.size() > 0 && path[0] == '/') {
    result = '/' + result;  // leading slash
  }
  return result;
}

bool FileValidater::IsPathUnderDocumentRoot(const std::string& path,
                                            const std::string& document_root) {
  if (document_root.empty()) return false;
  if (document_root == "/") return true;  // root matches all
  if (path.size() < document_root.size()) return false;
  if (path.compare(0, document_root.size(), document_root) != 0) return false;
  if (path.size() == document_root.size()) return true;  // exact match
  return path[document_root.size()] == '/';              // boundary check
}

// path always starts with "/" (absolute path)
bool FileValidater::IsValidFilePath(const std::string& path,
                                    const std::string& document_root) {
  if (ContainsUnsafeChars(path)) return false;
  std::string normalized_path = NormalizeSlashes(path);
  normalized_path = RemoveSingleDotSegments(normalized_path);
  if (ContainsDotDotSegments(normalized_path)) return false;
  if (!IsPathUnderDocumentRoot(normalized_path, document_root)) return false;
  return true;
}
