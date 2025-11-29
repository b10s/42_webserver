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
 // TODO: maybe I should not throw bad request for extensions(trailing section)
 // but just ignore them. For now, we just throw bad request for simplicity.

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
  const size_t remaining = need - content_received_; // remaining bytes to read
  if (buffer_.empty()) return false;
  const size_t to_read = std::min(remaining, buffer_.size());
  body_.append(buffer_, 0, to_read);
  buffer_.erase(0, to_read);
  content_received_ += to_read;
  if (content_received_ < need) {
    return false;  // need more data
  }
  if (content_received_ > need) {
    // should not happen if we manage buffer_ correctly
    throw http::ResponseStatusException(kInternalServerError);
  }
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
    if (chunk_size == 0) { // parseChunkSize advanced pos past CRLF
      return ValidateFinalCRLF(pos);
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
  pos += 2; // skip CRLF
  return true;
}

// called after reading "0\r\n"; now expect the final CRLF
bool HttpRequest::ValidateFinalCRLF(size_t& pos) {
  if (pos + 1 >= buffer_.size()) return false;  // still waiting the final CRLF
  if (buffer_[pos] != '\r' || buffer_[pos + 1] != '\n') {
    throw http::ResponseStatusException(kBadRequest);
  }
  pos += 2;  // skip final CRLF
  if (pos != buffer_.size()) {
    throw http::ResponseStatusException(kBadRequest);
  }
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
