#ifndef CGIEXECUTOR_HPP_
#define CGIEXECUTOR_HPP_

#include <unistd.h>

#include <map>
#include <string>
#include <vector>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Location.hpp"
#include "lib/type/Optional.hpp"

// RFC 3875
// https://tex2e.github.io/rfc-translater/html/rfc3875.html
//
// ### CGIレスポンスの解析 (parse_cgi_and_build_http_response) ###
//
// 1. スクリプトの出力を最初の空行で「ヘッダ部」と「ボディ部」に分離する
// 2. ヘッダ部を解析する
//    - "Location:" ヘッダがある場合:
//      -> リダイレクト用のHTTPレスポンス (302 Foundなど) を生成する
//    - "Status:" ヘッダがある場合:
//      -> 指定されたステータスコードでHTTPレスポンスを生成する
//    - "Content-Type:" ヘッダがある場合 (通常):
//      -> 200
//      OKのHTTPレスポンスを生成し、Content-Typeヘッダとボディ部を設定する
//    - その他のヘッダ (例: Set-Cookie):
//      -> そのままHTTPレスポンスヘッダとして追加する
// 3. 組み立てたHTTPレスポンスを返す

class CgiExecutor {
 private:
  // Prohibit to use default constructor
  CgiExecutor();

  enum { kReadEnd = 0, kWriteEnd = 1 };

  std::map<std::string, lib::type::Optional<std::string> > meta_vars_;
  std::string GetMetaVar(const std::string&) const;
  std::vector<std::string> GetMetaVars() const;
  void InitializeMetaVars(const HttpRequest&);

  std::string script_path_;

 public:
  CgiExecutor(const HttpRequest&, const std::string&);
  ~CgiExecutor();

  HttpResponse Run();
};

#endif
