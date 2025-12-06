#include "HttpRequest.hpp"

// \r\n\r\n indicates the end of the header section
// size_t and size_type are used to avoid signed/unsigned comparison warnings
// size_type is used here because string::npos is of that type
std::string::size_type HttpRequest::FindEndOfHeader(
    const std::string& payload) {
  static const std::string kDelimiter = "\r\n\r\n";
  std::string::size_type pos = payload.find(kDelimiter);
  if (pos != std::string::npos) {
    return pos + kDelimiter.size();  // position after the kDelimiter
  }
  return std::string::npos;  // not found
}

/**
 * @brief ヘッダー終了文字列（"\r\n\r\n"）が現れるまで待機し、
 * 揃った時点でリクエストライン（メソッド・URI・バージョン）と
 * 各ヘッダー行を順に解析する。
 *
 * @return bool
 *         - true: ヘッダー解析が完了　progress = BODY に更新
 *         - false: ヘッダーが未完（再試行）
 *
 * @throw lib::exception::ResponseStatusException
 *        - BAD_REQUEST: 不正な構文
 *        - INTERNAL_SERVER_ERROR: 想定外の例外
 *
 * @post
 *  - buffer_ から消費済みデータが削除される
 *  - progress が BODY に更新される（ヘッダー完了時）
 */
bool HttpRequest::AdvanceHeaderParsing() {
  std::string::size_type end_of_header = FindEndOfHeader(buffer_);
  if (end_of_header == std::string::npos) {
    return false;  // need more data
  }
  std::string header_section =
      buffer_.substr(0, end_of_header);  // includes "\r\n\r\n"
  try {
    const char* cur = header_section.c_str();
    cur = this->ConsumeMethod(cur);
    cur = this->ConsumeUri(cur);
    cur = this->ConsumeVersion(cur);
    cur = this->ConsumeHeader(cur);
    (void)cur;  // suppress unused variable warning
  } catch (lib::exception::ResponseStatusException& e) {
    throw;
  } catch (std::exception&) {
    throw lib::exception::ResponseStatusException(
        lib::http::kInternalServerError);
  }
  buffer_ = buffer_.substr(end_of_header);
  progress_ = kBody;
  return true;
}
