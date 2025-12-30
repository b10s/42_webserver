#ifndef FILEVALIDATER_HPP_
#define FILEVALIDATER_HPP_
#include <string>

class FileValidator {
public:
    static bool IsValidPath(const std::string& path);  // this issue
    static std::string SanitizePath(const std::string& path);
    static bool IsPathUnderDocumentRoot(const std::string& path, 
                                       const std::string& document_root);
    static bool ContainsDangerousPattern(const std::string& path);
    
private:
    static std::string NormalizePath(const std::string& path);
    static std::string ResolvePath(const std::string& path);
};

#endif  // FILEVALIDATER_HPP_
