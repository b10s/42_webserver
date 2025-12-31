#ifndef FILEVALIDATER_HPP_
#define FILEVALIDATER_HPP_
#include <string>

class FileValidator {
public:
    static bool IsValidPath(const std::string& path);
    static std::string SanitizePath(const std::string& path);
    static bool IsPathUnderDocumentRoot(const std::string& path, 
                                       const std::string& document_root);
    static bool ContainsDangerousPattern(const std::string& path);
    
private:
    static std::string NormalizePath(const std::string& path);
    static std::string ResolvePath(const std::string& path);
};

#endif  // FILEVALIDATER_HPP_

/*
config：実装をシンプルにするため厳しくしている
実行時入力（URI）：互換性のため 表記は緩める、その代わり docroot 逸脱は絶対に防ぐ

緩める（＝普通に通したい）
- URI 中の /（パス区切り）は当然許可
- %xx の percent-encoding を許可（その後 decode して扱う）
厳しくする（＝ここが本丸）
- decode 後に NUL(\0) / 制御文字は拒否
- .. は path segment として拒否（/a/../b や %2e%2e も）
- root + remainder を作ったあと、正規化（canonicalize）して docroot 配下かチェック（これが “本当に逃げられない” ガード）

共通化できそうな部分
lib::utils に **“判断しない部品”**だけ置く
- ContainsCtlOrWhitespace(s)
- ContainsNul(s)
- ContainsDotDot(s)（ただし URI 用は “segment 判定”の版も欲しい）

上位で用途別に組み立てる
- ValidateIndexFilename(token)（厳しい allowlist）
- ValidateUriPath(decoded_path)（/ OK、percent decode 後に segment チェック等）
- ResolveAndCheckUnderDocroot(root, path)（正規化して docroot 判定）

「正規化（canonicalize）」とは？
- パスを 標準的な形式に変換すること
- /../の解消（親ディレクトリに戻す）
- 余分な / の削除　パスの途中に / が連続している場合、1つの / にまとめる
- symlink の解消（シンボリックリンクをたどる） file system レベルでの解決
- 例: /var/www/html/../uploads//file.txt  ->  /var/www/uploads/file.txt

*/
