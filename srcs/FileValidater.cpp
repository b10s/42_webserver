#include "FileValidater.hpp"
#include "lib/http/CharValidation.hpp"
#include <iostream>

/*
仕様
percent-decode しない
NUL/制御文字拒否 : lib/http/CharValidation.hpp の IsValidHeaderChar() を流用

ContainsDotDotSegment(path): 
- 1回でも出たら即 reject（400/404）
- (移動するならstackやdequeで管理したほうがよいかも？いったん保留)
- reject:`/../a`  `/a/../b` `/a/..` / `..`
- rejectしない例（ファイル名）:  `/a..b` / `/a/..b` / `/a/b..`

IsUnderDocroot(root + remainder, docroot)（境界チェック付き）
NormalizeLexically()  "//"と"."を畳むだけ
*/

// Check for spaces or control characters
bool FileValidator::ContainsUnsafeChars(const std::string& path) {
    for (size_t i = 0; i < path.size(); ++i) {
        char c = path[i];
        if (!lib::http::IsValidHeaderChar(c)) {
            return true;  // contains NUL or control character - throw??
        }
    }
    return false;
}

// reject: "/a/../b", "/..", "/a/..", "../a", ".."
bool FileValidator::ContainsDotDotSegments(const std::string& path) {
  for (size_t i = 0; (i = path.find("..", i)) != std::string::npos; ++i) {
    bool left_ok  = (i == 0) || (path[i - 1] == '/');
    bool right_ok = (i + 2 == path.size()) || (path[i + 2] == '/');
    if (left_ok && right_ok) return true;
  }
  return false;
}

// "/////" を "/" に変換
std::string FileValidator::NormalizeSlashes(const std::string& path) {
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
// もし..を今後移動に使うなら stack/deque にするかもしれない
std::vector<std::string> FileValidator::SplitPathSegments(const std::string& path) {
    std::vector<std::string> segments;
    size_t start = 0;
    size_t end = 0;
    while ((end = path.find('/', start)) != std::string::npos) {
        if (end != start) { // avoid empty segment
            segments.push_back(path.substr(start, end - start));
        }
        start = end + 1;
    }
    if (start < path.size()) { // last segment
        segments.push_back(path.substr(start));
    }
    return segments;
}

std::string FileValidator::RemoveSingleDotSegments(const std::string& path) {
    std::string result;
    std::vector<std::string> segments = SplitPathSegments(path);
    for (size_t i = 0; i < segments.size(); ++i) {
        if (segments[i] == ".") continue ; // skip
        if (!result.empty()) result += '/';
        result += segments[i];
    }
    if (path.size() > 0 && path[0] == '/') {
        result = '/' + result; // leading slash
    }
    return result;
}

// I don't think we need to resolve symlinks in this project
bool FileValidator::IsPathUnderDocumentRoot(const std::string& path, 
                                            const std::string& document_root) {
    if (document_root.empty()) {
        return false;
    }
    if (path.size() < document_root.size()) {
        return false;
    }
    if (path.compare(0, document_root.size(), document_root) != 0) {
        return false;
    }
    if (path.size() == document_root.size()) { // exact match
        return true;
    } 
    if (document_root[document_root.size() - 1] == '/') {
        return true; // document_root ends with '/', so path is under it
    }
    return path[document_root.size()] == '/'; // boundary check
}

bool FileValidator::IsValidFilePath(const std::string& path, 
                                     const std::string& document_root) {
    if (ContainsUnsafeChars(path)) {
        return false;
    }
    if (ContainsDotDotSegments(path)) {
        return false;
    }
    std::string normalized_path = NormalizeSlashes(path);
    normalized_path = RemoveSingleDotSegments(normalized_path);

    // std::cerr
    //     << "[FileValidator] path=" << path
    //     << " root=" << document_root
    //     << " unsafe=" << ContainsUnsafeChars(path)
    //     << " dotdot=" << ContainsDotDotSegments(path)
    //     << " normalized=" << normalized_path
    //     << " under=" << IsPathUnderDocumentRoot(normalized_path, document_root)
    //     << "\n";

    // if (ContainsUnsafeChars(path)) return false;
    // if (ContainsDotDotSegments(path)) return false;
    if (!IsPathUnderDocumentRoot(normalized_path, document_root)) return false;
    return true;
}
