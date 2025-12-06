#include "HttpRequest.hpp"

// \r\n\r\n indicates the end of the header section
// size_t and size_type are used to avoid signed/unsigned comparison warnings
// size_type is used here because string::npos is of that type
std::string::size_type HttpRequest::FindEndOfHeader(const std::string& payload)
{
  const std::string delimiter = "\r\n\r\n";
  const size_t len = payload.size();

  for (size_t i = 0; i + 3 < len; i++)
  {
    if (payload.compare(i, delimiter.size(), delimiter) == 0){
      return i + delimiter.size(); // position after the delimiter
    }
  }
  return std::string::npos; // not found
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
 * @throw http::responseStatusException
 *        - BAD_REQUEST: 不正な構文
 *        - INTERNAL_SERVER_ERROR: 想定外の例外
 *
 * @post
 *  - buffer_ から消費済みデータが削除される
 *  - progress が BODY に更新される（ヘッダー完了時）
 */
bool HttpRequest::AdvanceHeaderParsing() {
  std::string::size_type endOfHeader = FindEndOfHeader(buffer_);
  if (endOfHeader == std::string::npos) {
    return false; // need more data
  }
  std::string headerSection = buffer_.substr(0, endOfHeader); // includes "\r\n\r\n"
  try {
    const char* cur = headerSection.c_str();
    cur = this->ConsumeMethod(cur);
    cur = this->ConsumeUri(cur);
    cur = this->ConsumeVersion(cur);
    cur = this->ConsumeHeader(cur);
  } catch (lib::exception::ResponseStatusException& e) { 
    throw; 
  } catch (std::exception&) {
    throw lib::exception::ResponseStatusException(lib::http::kInternalServerError);
  }
  buffer_ = buffer_.substr(endOfHeader);
  progress_ = kBody;
  return true;
}

