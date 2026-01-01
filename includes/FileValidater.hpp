#ifndef FILEVALIDATOR_HPP_
#define FILEVALIDATOR_HPP_
#include <string>
#include <vector>

/*
config：実装をシンプルにするためルールは厳しくする。
実行時入力（URI）：互換性のためconfigよりゆるくする。

configより緩い部分
- URI 中の /（パス区切り）は当然許可
厳しい部分
- .. は path segment として拒否
- root + remainder -> 正規化 -> docroot
配下かチェック
*/

class FileValidator {
 public:
  // made public for testing
  static bool ContainsDotDotSegments(const std::string& path);
  static std::string NormalizeSlashes(const std::string& path);
  static std::vector<std::string> SplitPathSegments(const std::string& path);
  static std::string RemoveSingleDotSegments(const std::string& path);
  static bool IsPathUnderDocumentRoot(const std::string& path,
                                      const std::string& document_root);

  static bool IsValidFilePath(const std::string& path,
                              const std::string& document_root);

 private:
  static bool ContainsUnsafeChars(const std::string& path);
};

#endif  // FILEVALIDATOR_HPP_
