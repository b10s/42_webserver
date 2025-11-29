#include "HttpRequest.hpp"

/**
 * @brief handle body parsing advancement, either by content-length or chunked
 *
 * @return bool
 *         - true: finished（progress = kDone ）
 *         - false: not enough data, progress remains kBody, next call needed
 *
 * @throw http::responseStatusException
 *        - BAD_REQUEST: malformed chunked encoding
 *        - INTERNAL_SERVER_ERROR: unexpected internal error
 *
 * @post
 *  - body_: appended with newly parsed body data
 *  - progress_: updated to kDone if finished
 *  - buffer_: updated to remove consumed data
 */

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
  buffer_.erase(0, need);
  progress_ = kDone;
  return true;
}

// chunked transfer encoding: "size\r\n<data>\r\n ... 0\r\n\r\n"
// return false if need more data
bool HttpRequest::AdvanceChunkedBody() {
  size_t pos = 0;
  for (;;) {
    size_t chunk_size = 0;
    if (!ParseChunkSize(pos, chunk_size)) {
      return false;
    }
    if (chunk_size == 0) {
      return HandleLastChunk(pos);
    }
    if (!AppendChunkData(pos, chunk_size)) {
      return false;
    }
    // pos is advanced, but buffer_ is not yet erased
  }
}

// chunck size: <hex>\r\n
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
      // disallow extensions(';' is BAD_REQUEST)
      throw http::ResponseStatusException(kBadRequest);  
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

// "0\r\n\r\n"
bool HttpRequest::HandleLastChunk(size_t& pos) {
  if (pos + 1 >= buffer_.size()) return false;  // still waiting for "\r\n\r\n"
  if (buffer_[pos] != '\r' || buffer_[pos + 1] != '\n') {
    throw http::ResponseStatusException(kBadRequest);
  }
  pos += 2;
  // if (buffer_[pos] != '\r' || buffer_[pos + 1] != '\n') {
  //   throw http::ResponseStatusException(kBadRequest);
  // }
  // pos += 2;
  // if (pos > buffer_.size()) {
  //   throw http::ResponseStatusException(kBadRequest); // extra string after last chunk
  // }
  buffer_.erase(0, pos);  // erase consumed data including last chunk
  progress_ = kDone;
  return true;
}

// append "<data>\r\n" and advance pos, but do not erase yet
bool HttpRequest::AppendChunkData(size_t& pos, size_t chunk_size) {
  const size_t total_needed = chunk_size + 2;
  if (pos + total_needed > buffer_.size()) {
    return false;
  }
  body_.append(buffer_, pos, chunk_size);
  pos += chunk_size;
  if (buffer_[pos] != '\r' || buffer_[pos + 1] != '\n') {
    throw http::ResponseStatusException(kBadRequest);
  }
  pos += 2;
  return true;
}

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
