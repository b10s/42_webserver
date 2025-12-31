#include "FileValidater.hpp"
#include "lib/http/CharValidation.hpp"
#include <iostream>

// std::string FileValidator::NormalizePath(const std::string& path) {
//     std::string normalized;
//     size_t i = 0;
//     while (i < path.size()) {
//         // Skip consecutive slashes
//         if (path[i] == '/') {
//             normalized += '/';
//             while (i < path.size() && path[i] == '/') {
//                 ++i;
//             }
//             continue;
//         }

//         // Handle "." segments
//         if (path.compare(i, 2, "./") == 0 || 
//             (i + 1 == path.size() && path[i] == '.')) {
//             i += 2;  // skip "./"
//             continue;
//         }

//         // Handle ".." segments
//         if (path.compare(i, 3, "../") == 0 || 
//             (i + 2 == path.size() && path.compare(i, 2, "..") == 0)) {
//             // Remove the last segment from normalized path
//             size_t pos = normalized.find_last_of('/', normalized.size() - 2);
//             if (pos != std::string::npos) {
//                 normalized.erase(pos + 1);
//             } else {
//                 normalized.clear();
//             }
//             i += 3;  // skip "../"
//             continue;
//         }

//         // Copy normal characters
//         size_t start = i;
//         while (i < path.size() && path[i] != '/') {
//             ++i;
//         }
//         normalized.append(path, start, i - start);
//     }

//     // Remove trailing slash if not root
//     if (normalized.size() > 1 && normalized[normalized.size() - 1] == '/') {
//         normalized.erase(normalized.size() - 1);
//     }

//     return normalized;
// }
// https://github.com/kurrrru/webserv/blob/master/src/http/request/request_parser.cpp
// void RequestParser::normalizationPath() {
//     std::string normalizePath = _request.uri.path;
//     std::size_t slashPos = 0;
//     while ((slashPos = normalizePath.find("//", slashPos)) !=
//            std::string::npos) {
//         normalizePath.replace(slashPos, parser::HEX_DIGIT_LENGTH, "/");
//     }
//     std::string parts;
//     for (std::size_t i = 0; i < normalizePath.size(); ++i) {
//         if (normalizePath[i] == '/' && !parts.empty()) {
//             _request.uri.splitPath.push_back(parts);
//             parts.clear();
//         }
//         parts += normalizePath[i];
//     }
//     if (!parts.empty()) {
//         _request.uri.splitPath.push_back(parts);
//     }
// }

// void RequestParser::verifySafePath() {
//     std::deque<std::string> pathDeque;
//     for (std::size_t i = 0; i < _request.uri.splitPath.size(); ++i) {
//         if (_request.uri.splitPath[i] == "/..") {
//             if (pathDeque.empty()) {
//                 toolbox::logger::StepMark::error(
//                     "RequestParser: invalid path, try parent directory");
//                 _request.httpStatus.set(HttpStatus::BAD_REQUEST);
//                 return;
//             } else {
//                 pathDeque.pop_front();
//             }
//         } else if (_request.uri.splitPath[i] != "/.") {
//             pathDeque.push_front(_request.uri.splitPath[i]);
//         }
//     }
//     _request.uri.path.clear();
//     std::size_t totalLength = calcTotalLength(pathDeque);
//     _request.uri.path.reserve(totalLength);
//     for (int i = pathDeque.size() - 1; i >= 0; --i) {
//         _request.uri.path += pathDeque[i];
//     }
// }

// Check for spaces or control characters (e.g., NULL byte, etc.)
bool FileValidator::ContainsUnsafeChars(const std::string& path) {
    for (size_t i = 0; i < path.size(); ++i) {
        char c = path[i];
        if (c == '\0' || (c >= 0 && c <= 31) || c == 127) {
            return true;  // contains NUL or control character
        }
    }
    return false;
}

// I don't think we have to resolve symlinks
bool FileValidator::IsPathUnderDocumentRoot(const std::string& path, 
                                            const std::string& document_root) {
    if (path.compare(0, document_root.size(), document_root) != 0) {
        return false;
    }
    return true;
}

bool FileValidator::IsValidFilePath(const std::string& path, 
                                     const std::string& document_root) {
    // Check for unsafe characters
    if (ContainsUnsafeChars(path)) {
        return false;
    }

    // Check for ".." segments (path traversal: /a/../b or ../b)
    size_t pos = 0;
    while ((pos = path.find("../", pos)) != std::string::npos) {
        return false;  // TODO: trail ".." case
    }

    // Check if the path is under the document root
    if (!IsPathUnderDocumentRoot(path, document_root)) {
        return false;
    }

    return true;
}
