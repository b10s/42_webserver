#include "HttpRequest.hpp"

bool HttpRequest::AdvanceBodyParsing() {
  try {
    if (content_length_ >= 0) {
      return AdvanceContentLengthBody();
    } else {
      return AdvanceChunkedBody();
    }
  } catch (http::ResponseStatusException&) {
    throw;  // rethrow
  } catch (...) {
    throw http::ResponseStatusException(kInternalServerError);
  }
}

// content length mode
bool HttpRequest::AdvanceContentLengthBody() {
  const size_t need = static_cast<size_t>(content_length_);
  if (buffer_.size() < need) return false;  // need more data
  body_.assign(buffer_.data(), need);
  buffer_.erase(0, need);  // ← 忘れずに消費済みデータを削除
  progress_ = kDone;
  return true;
}

// chunked transfer encoding: return false if more data needed
bool HttpRequest::AdvanceChunkedBody() {
  // chunked: "size\r\n<data>\r\n ... 0\r\n\r\n"
  size_t pos = 0;
  for (;;) {
    // 1) サイズ行を16進で読む（拡張は未対応：';' は許容しない）
    size_t chunk_size = 0;
    if (!ParseChunkSize(pos, chunk_size)) {
      return false;  // need more data or error
    }
    // 2) 終端チャンク
    if (chunk_size == 0) {
      return HandleLastChunk(pos);  // false if need more data
    }
    if (!AppendChunkData(pos, chunk_size)) {
      return false;  // need more data or error
    }
    // 次のサイズ行へ　（まだeraseしないでposのみ前進）
  }
}

/// 1) サイズ行を16進で読む（拡張は未対応：';' は許容しない）
// <hex>\r\n
bool HttpRequest::ParseChunkSize(size_t& pos, size_t& chunk_size) {
  chunk_size = 0;
  bool saw_digit = false;
  while (pos < buffer_.size()) {
    char c = buffer_[pos];
    if (c == '\r') break;
    if (c >= '0' && c <= '9') {
      chunk_size = (chunk_size << 4) + (c - '0');
      saw_digit = true;
    } else if (c >= 'a' && c <= 'f') {
      chunk_size = (chunk_size << 4) + (c - 'a' + 10);
      saw_digit = true;
    } else if (c >= 'A' && c <= 'F') {
      chunk_size = (chunk_size << 4) + (c - 'A' + 10);
      saw_digit = true;
    } else {
      throw http::ResponseStatusException(
          kBadRequest);  // 拡張 (";ext") などは未対応 → BAD_REQUEST
    }
    ++pos;
  }
  if (!saw_digit) return false;
  if (pos + 1 >= buffer_.size()) return false;  // needs '\n' after '\r'
  if (buffer_[pos] != '\r' || buffer_[pos + 1] != '\n')
    throw http::ResponseStatusException(kBadRequest);
  pos += 2;
  return true;
}

// this function is called when chunk_sie is zero
// "0\r\n\r\n" が揃っていたら消費してkDoneにする
bool HttpRequest::HandleLastChunk(size_t& pos) {
  if (pos + 1 >= buffer_.size()) return false;  // still waiting for "\r\n"
  if (buffer_[pos] != '\r' || buffer_[pos + 1] != '\n') return false;
  pos += 2;
  buffer_.erase(0, pos);  // サイズ行＋CRLFを消費
  progress_ = kDone;
  return true;
}

// chunk_size バイトのデータ＋末尾のCRLFを body_ に追加する
// pos はその次に進める
bool HttpRequest::AppendChunkData(size_t& pos, size_t chunk_size) {
  const size_t total_needed = chunk_size + 2;  // data + CRLF
  if (pos + total_needed > buffer_.size()) {
    return false;  // need more data
  }
  // 5) 本体を body_ に追加
  body_.append(buffer_, pos, chunk_size);
  pos += chunk_size;
  // 6) 本体直後のCRLF確認
  if (buffer_[pos] != '\r' || buffer_[pos + 1] != '\n') {
    throw http::ResponseStatusException(kBadRequest);
  }
  pos += 2;
  return true;
}

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
// bool HttpRequest::AdvanceBodyParsing() {
//   // content length mode
//   if (content_length_ >= 0) {
//     const size_t need = static_cast<size_t>(content_length_);
//     if (buffer_.size() < need) return false; // need more data
//     body_.assign(buffer_.data(), need);
//     buffer_.erase(0, need);    // ← 忘れずに消費済みデータを削除
//     progress_ = kDone;
//     return true;
//   }
//   // chunked transfer encoding: return false if more data needed
//   // hex chunk size followed by CRLF, then chunk data, then CRLF
//   // chunked: "size\r\n<data>\r\n ... 0\r\n\r\n"
//   size_t pos = 0;
//   for (;;) {
//     // 1) サイズ行を16進で読む（拡張は未対応：';' は許容しない）
//     size_t chunk_size = 0;
//     bool saw_digit = false;
//     while (pos < buffer_.size()) {
//       char c = buffer_[pos];
//       if (c == '\r') break;
//       if (c >= '0' && c <= '9') {chunk_size = (chunk_size << 4) + (c - '0');
//       saw_digit = true; } else if (c >= 'a' && c <= 'f') {chunk_size =
//       (chunk_size << 4) + (c - 'a' + 10); saw_digit = true; } else if (c >=
//       'A' && c <= 'F') {chunk_size = (chunk_size << 4) + (c - 'A' + 10);
//       saw_digit = true; } else {
//         throw http::ResponseStatusException(kBadRequest); // ';' は
//         BAD_REQUEST
//       }
//       ++pos;
//     }
//     if (!saw_digit) return false;
//     // 2) サイズ行のCRLF
//     // チャンクサイズを読み終わった後の状態
//     // buffer_: "5\r\nhello\r\n0\r\n\r\n"
//     //            ^
//     //           pos (ここが '\r')
//     if (pos + 1 >= buffer_.size()) return false; // need more data
//     if (buffer_[pos] != '\r' || buffer_[pos + 1] != '\n')
//       throw http::ResponseStatusException(kBadRequest);
//     pos += 2;

//     // 3) 終端チャンク
//     if (chunk_size == 0) {
//       // 直後に CRLF を期待（trailer 未対応）
//       if (pos + 1 >= buffer_.size()) return false; // need more data
//       if (buffer_[pos] != '\r' || buffer_[pos + 1] != '\n')
//         return false;
//       pos += 2;
//       buffer_.erase(0, pos); // サイズ行＋CRLFを消費
//       progress_ = kDone;
//       return true;
//     }
//     // 4) 本体 + CRLFが揃っているか
//     if (pos + chunk_size + 2 > buffer_.size()) {
//       return false; // need more data
//     }
//     // 5) 本体を body_ に追加
//     body_.append(buffer_, pos, chunk_size);
//     pos += chunk_size;
//     // 6) 本体直後のCRLF確認
//     if (buffer_[pos] != '\r' || buffer_[pos + 1] != '\n') {
//       throw http::ResponseStatusException(kBadRequest);
//     }
//     pos += 2;
//     // 次のサイズ行へ　（まだeraseしないでposのみ前進）
//   }
// }
