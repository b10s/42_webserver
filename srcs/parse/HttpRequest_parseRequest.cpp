#include "HttpRequest.hpp"

// \r\n\r\n indicates the end of the header section

// size_t を返すなら「見つかったときはインデックス
// 無いときは std::string::npos」など契約をコメントで明示。
// 戻り値型は std::string::size_type が自然です。
std::string::size_type HttpRequest::find_end_of_header(const std::string& payload)
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
  std::string::size_type endOfHeader = find_end_of_header(buffer_);
  if (endOfHeader == std::string::npos) {
    return false; // need more data
  }
  std::string headerSection = buffer_.substr(0, endOfHeader - 4); // maybe unnecessary...?
  try {
    const char* cur = headerSection.c_str();
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
      // consume trailing CRLF after last chunk
      // 実装では trailer 未対応：直後に CRLF が来る想定（"0\r\n\r\n"）
      if (static_cast<size_t>(buffer_.end() - it) < 2) return false;
      if (*it != '\r' || *(it + 1) != '\n') throw http::responseStatusException(BAD_REQUEST);
      it += 2;
      // ここまで処理したぶんを buffer_ から取り除く
      const std::string::size_type consumed =
      static_cast<std::string::size_type>(it - buffer_.begin());
      buffer_.erase(0, consumed);
      progress = DONE;
      return true;
    }
    std::string::const_iterator it = buffer_.begin();
    std::string::const_iterator end = buffer_.end();
    if (static_cast<size_t>(end - it) < chunkSize + 2) {
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
