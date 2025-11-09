#include "HttpRequest.hpp"

/**
 * @brief Content-Length または chunked transfer encoding に対応する。
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
bool HttpRequest::advanceBodyParsing() {
  if (contentLength_ >= 0) {
    if (buffer_.size() < static_cast<size_t>(contentLength_)) {
      return false; // need more data
    }
    body_ = buffer_.substr(0, contentLength_); // extract body
    // body_.assign(buffer_.data(), need);
    // buffer_.erase(0, need);    // ← 忘れずに消費済みデータを削除
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

//////////////////////////////////////////////////////////////////////////////////////////////////

// parseBodyStep() の安全化（バグ修正を含む）
// あなたの実装には次の問題がありました：
// Content-Length のとき buffer_ を消費していない
// 取り出した後に buffer_.erase(0, contentLength_) が必要。
// chunked で iterator をシャドーしている
// 途中で it を再宣言していてロジックが壊れやすい。
// chunked のサイズ行の CRLF までしか確認していない
// データ本体 chunkSize バイト＋直後の CRLF の 2 バイト、両方あるかを確認すべき。
// 終端チャンク “0\r\n\r\n” 後の消費量計算が不安定
// 走査はインデックス（size_t pos）で行うと安全。

// bool HttpRequest::advanceBodyParsing() {
//   // Content-Length
//   if (contentLength_ >= 0) {
//     const size_t need = static_cast<size_t>(contentLength_);
//     if (buffer_.size() < need) return false;
//     body_.assign(buffer_.data(), need);
//     buffer_.erase(0, need);    // ← 忘れずに消費
//     progress = DONE;
//     return true;
//   }

//   // chunked (trailer 未対応: "0\r\n\r\n" を期待)
//   size_t pos = 0;
//   for (;;) {
//     // 1) サイズ行（16進数）を読む
//     size_t chunkSize = 0;
//     bool sawDigit = false;
//     while (pos < buffer_.size()) {
//       char c = buffer_[pos];
//       if (c == '\r') break;
//       if (c >= '0' && c <= '9') { chunkSize = (chunkSize << 4) + (c - '0'); sawDigit = true; }
//       else if (c >= 'a' && c <= 'f') { chunkSize = (chunkSize << 4) + (c - 'a' + 10); sawDigit = true; }
//       else if (c >= 'A' && c <= 'F') { chunkSize = (chunkSize << 4) + (c - 'A' + 10); sawDigit = true; }
//       else {
//         throw http::responseStatusException(BAD_REQUEST);
//       }
//       ++pos;
//     }
//     if (!sawDigit) return false; // サイズがまだ来ていない

//     // 2) サイズ行の CRLF
//     if (pos + 1 >= buffer_.size()) return false;
//     if (buffer_[pos] != '\r' || buffer_[pos + 1] != '\n')
//       throw http::responseStatusException(BAD_REQUEST);
//     pos += 2;

//     // 3) 終端チャンク
//     if (chunkSize == 0) {
//       // "0\r\n\r\n" を期待（trailer 未対応）
//       if (pos + 1 >= buffer_.size()) return false;
//       if (buffer_[pos] != '\r' || buffer_[pos + 1] != '\n')
//         return false; // まだ最後の CRLF が来ていない
//       pos += 2;

//       // ここまでが消費済み
//       buffer_.erase(0, pos);
//       progress = DONE;
//       return true;
//     }

//     // 4) チャンク本体 + CRLF の有無確認
//     if (pos + chunkSize + 2 > buffer_.size()) return false; // 未到着
//     // 本体を body_ へ追加
//     body_.append(buffer_, pos, chunkSize);
//     pos += chunkSize;

//     if (buffer_[pos] != '\r' || buffer_[pos + 1] != '\n')
//       throw http::responseStatusException(BAD_REQUEST);
//     pos += 2;

//     // ループ継続（次のサイズ行へ）
//     // ここではまだ erase しない（`pos` で前進管理）
//   }
// }
