#include "HttpRequest.hpp"

// \r\n\r\n indicates the end of the header section
static size_t is_end_of_header(const std::string& payload)
{
  const std::string delimiter = "\r\n\r\n";
  const size_t len = payload.size();

  for (size_t i = 0; i + 3 < len; i++)
  {
    if (payload.compare(i, delimiter.size(), delimiter) == 0){
      return i + delimiter.size(); // position after the delimiter
    }
  }
  return 0; // not found yet
}

/**
 * @brief HTTPリクエスト全体を段階的に解析するメイン関数。
 *
 * 受信データ（payload）を内部バッファに追加し、
 * 現在の解析状態（HEADER / BODY / DONE）に応じて
 * 適切なステップ関数（parseHeaderStep または parseBodyStep）を呼び出す。
 * 
 * @param payload 新しく受信したデータ（NULL終端文字列）
 * 
 * @throw http::responseStatusException
 *        - BAD_REQUEST: リクエスト完了後にさらに入力が来た場合など
 *        - INTERNAL_SERVER_ERROR: 解析処理中の想定外エラー
 * 
 * @note
 * - 不足データの場合は途中で return し、次回以降に続きを解析する。
 * - 完了時（DONE）はリクエスト全体のパースが成功した状態。
 */
void HttpRequest::parseRequest(const char* payload) {
  try {
    buffer_ += payload;

    for (;;) {
      switch (progress) {
        case HEADER:
          if (!consumeHeader()) return; // still need more data
          continue; // after header is parsed, re-evaluate progress

        case BODY:
          if (!consumeBody()) return;
          return; // if body is parsed, progress is set to DONE
        case DONE:
          throw http::responseStatusException(BAD_REQUEST);
      }
    }
  }
  catch (http::responseStatusException& e) { throw; }
  catch (std::exception&) {
    throw http::responseStatusException(INTERNAL_SERVER_ERROR);
  }
}

/**
 * @brief HTTPリクエストのヘッダー部をを1ステップずつ解析する。
 * ヘッダー終了文字列（"\r\n\r\n"）が現れるまで待機し、
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
bool HttpRequest::consumeHeader() {
  size_t endOfHeader = is_end_of_header(buffer_);
  if (!endOfHeader) {
    return false; // need more data
  }
  try {
    const char* cur = buffer_.c_str();
    cur = this->parseMethod(cur);
    cur = this->parseUri(cur);
    cur = this->parseVersion(cur);
    cur = this->parseHeader(cur);
  } catch (http::responseStatusException& e) { 
    throw; 
  } catch (std::exception&) {
    throw http::responseStatusException(INTERNAL_SERVER_ERROR);
  }
  buffer_ = buffer_.substr(endOfHeader);
  progress = BODY;
  return true;
}

/**
 * @brief HTTPリクエストの本文（body）を1ステップずつ解析する。
 * Content-Length または chunked transfer encoding に対応する。
 * まだbodyが揃っていない場合は false を返し、揃い次第 true を返す。
 *
 * @return bool
 *         - true: 解析完了（progress = DONE）
 *         - false: 未完（再試行）
 *
 * @throw http::responseStatusException
 *        - BAD_REQUEST: 不正なチャンク形式や構文エラー
 *        - INTERNAL_SERVER_ERROR: 予期せぬ内部エラー
 *
 * @post
 *  - body_ に本文データが格納される
 *  - progress が DONE に更新される（完了時）
 *  - buffer_ から消費済みデータが削除される
 */
bool HttpRequest::consumeBody() {
  if (contentLength_ >= 0) {
    if (buffer_.size() < static_cast<size_t>(contentLength_)) {
      return false; // need more data
    }
    body_ = buffer_.substr(0, contentLength_); // extract body
    progress = DONE;
    return true;
  }
  // chunked transfer encoding: return false if more data needed
  // hex chunk size followed by CRLF, then chunk data, then CRLF
  std::string::const_iterator it = buffer_.begin();
  while (it != buffer_.end()) {
    size_t chunkSize = 0;
    while (it != buffer_.end() && *it != '\r') {
      if (*it >= '0' && *it <= '9') {
        chunkSize = chunkSize * 16 + *it - '0';
      } else if (*it >= 'a' && *it <= 'f') {
        chunkSize = chunkSize * 16 + *it - 'a' + 10;
      } else if (*it >= 'A' && *it <= 'F') {
        chunkSize = chunkSize * 16 + *it - 'A' + 10;
      } else {
        throw http::responseStatusException(BAD_REQUEST);
      }
      it++;
    }
    // no CRLF found
    if (it == buffer_.end() || *it != '\r' || (it + 1) == buffer_.end() || *(it + 1) != '\n') {
      return false; // need more data
    }
    it += 2; // CRLF
    // if chunk size is zero, we're done. no trailing headers are handled here.
    if (chunkSize == 0) {
      progress = DONE;
      return true;
    }
    if (static_cast<size_t>(std::distance(it, buffer_.end())) < chunkSize + 2) {
      return false; // not enough data for chunk + CRLF
    }
    // add chunk to body
    body_.append(it, it + chunkSize);
    it += chunkSize;
    // check for trailing CRLF
    if (it == buffer_.end() || *it != '\r' || *(it + 1) != '\n') {
      throw http::responseStatusException(BAD_REQUEST);
    }
    it += 2; // CRLF
    // update buffer to remove processed chunk
    buffer_ = buffer_.substr(it - buffer_.begin());
    it = buffer_.begin();
  }
  return false; // need more data, waiting for next chunk
}
