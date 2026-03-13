#include <iostream>
#include <limits>  // can't use SIZE_MAX in C++98 so use std::numeric_limits instead

#include "HttpRequest.hpp"

/**
 * @brief handle body parsing advancement, either by content-length or chunked
 *
 * @return bool
 *         - true: finished（progress = kDone ）
 *         - false: not enough data, progress remains kBody, next call needed
 *
 * @throw lib::exception::ResponseStatusException
 *        - BAD_REQUEST: malformed chunked encoding
 *        - INTERNAL_SERVER_ERROR: unexpected internal error
 *
 * @post
 *  - body_: appended with newly parsed body data
 *  - state_: updated to kDone if finished
 *  - buffer_: updated to remove consumed data
 */
// TODO: maybe I should not throw bad request for extensions(trailing section)
// but just ignore them. For now, we just throw bad request for simplicity.

static std::string EscapeForDebug(const std::string& s) {
  std::string out;
  for (size_t i = 0; i < s.size(); ++i) {
    if (s[i] == '\r')
      out += "\\r";
    else if (s[i] == '\n')
      out += "\\n";
    else
      out += s[i];
  }
  return out;
}

/*
ValidateBodyHeaders in ConsumeBodyHeaders ensures the following conditions:
  - If both Content-Length and Transfer-Encoding are present, throw BAD_REQUEST.
  - If no body is expected:
      content_length_ == 0 and there is no "transfer-encoding" header.
  - If Content-Length is present:
      content_length_ >= 0 and there is no "transfer-encoding" header.
  - If "Transfer-Encoding: chunked" is present:
      content_length_ == -1 and the "transfer-encoding" header exists.
*/
bool HttpRequest::AdvanceBody() {
  try {
    const bool has_transfer_encoding = headers_.count("transfer-encoding");
    if (content_length_ == 0 && !has_transfer_encoding) {
      state_ = kDone;
      return true;
    }
    if (content_length_ >= 0) {
      return AdvanceContentLengthBody();
    } else {
      return AdvanceChunkedBody();
    }
  } catch (lib::exception::ResponseStatusException&) {
    throw;
  } catch (...) {
    throw lib::exception::ResponseStatusException(
        lib::http::kInternalServerError);
  }
}

// content length mode
bool HttpRequest::AdvanceContentLengthBody() {
  const size_t need = static_cast<size_t>(content_length_);
  if (need > max_body_size_limit_) {
    throw lib::exception::ResponseStatusException(lib::http::kPayloadTooLarge);
  }
  if (need == 0) {
    state_ = kDone;
    return true;
  }
  if (buffer_.size() < need) {
    return false;
  }
  body_.assign(buffer_.data(), need);
  buffer_.erase(0, need);  // erase consumed data
  state_ = kDone;
  return true;
}

// chunked transfer encoding: "size\r\n<data>\r\n ... 0\r\n\r\n"
// return false if need more data
bool HttpRequest::AdvanceChunkedBody() {
  std::cerr << "[DEBUG chunk] enter AdvanceChunkedBody"
            << " state=" << state_ << " buffer_size=" << buffer_.size()
            << " read_pos=" << buffer_read_pos_
            << " next_chunk_size=" << next_chunk_size_ << std::endl;

  if (buffer_read_pos_ > buffer_.size()) {
    throw lib::exception::ResponseStatusException(
        lib::http::kInternalServerError);  // should not happen
  }
  if (buffer_read_pos_ == buffer_.size()) {
    std::cerr << "[DEBUG chunk] no more buffered data, need more" << std::endl;
    return false;  // parsed all available data, need more
  }
  for (;;) {
    std::cerr << "[DEBUG chunk] loop begin"
              << " read_pos=" << buffer_read_pos_
              << " next_chunk_size=" << next_chunk_size_ << std::endl;
    std::cerr << "[DEBUG chunk] remaining=["
              << EscapeForDebug(buffer_.substr(buffer_read_pos_)) << "]"
              << std::endl;
    if (next_chunk_size_ == -1) {
      size_t size = 0;
      size_t pos = buffer_read_pos_;
      std::cerr << "[DEBUG chunk] trying ParseChunkSize at pos=" << pos
                << std::endl;

      if (!ParseChunkSize(pos, size)) {
        std::cerr << "[DEBUG chunk] ParseChunkSize -> false (need more data)"
                  << std::endl;
        return false;  // need to wait for size line
      }
      std::cerr << "[DEBUG chunk] ParseChunkSize -> true"
                << " parsed_size=" << size << " new_pos=" << pos << std::endl;
      buffer_read_pos_ = pos;
      next_chunk_size_ = static_cast<ptrdiff_t>(size);
      // if size is 0, this is the last chunk, now expect final CRLF
      if (next_chunk_size_ == 0) {
        std::cerr << "[DEBUG chunk] last chunk detected (size 0)"
                  << " validate from pos=" << buffer_read_pos_ << std::endl;
        bool done = ValidateFinalCRLF(buffer_read_pos_);
        std::cerr << "[DEBUG chunk] ValidateFinalCRLF -> " << done
                  << " read_pos(after?)=" << buffer_read_pos_ << std::endl;
        if (done) {
          std::cerr << "[DEBUG chunk] request done, erasing consumed data"
                    << " erase_len=" << buffer_read_pos_ << std::endl;
          buffer_.erase(0, buffer_read_pos_);  // erase consumed data
          buffer_read_pos_ = 0;
          next_chunk_size_ = -1;
          state_ = kDone;
          return true;
        }
        return done;
      }
    }
    size_t pos = buffer_read_pos_;
    std::cerr << "[DEBUG chunk] trying AppendChunkData"
              << " pos=" << pos << " chunk_size=" << next_chunk_size_
              << std::endl;
    if (!AppendChunkData(pos, static_cast<size_t>(next_chunk_size_))) {
      std::cerr << "[DEBUG chunk] AppendChunkData -> false (need more data)"
                << std::endl;
      return false;
    }
    std::cerr << "[DEBUG chunk] AppendChunkData -> true"
              << " new_pos=" << pos << std::endl;
    buffer_read_pos_ = pos;
    next_chunk_size_ = -1;  // reset state, read next size line
    std::cerr << "[DEBUG chunk] chunk consumed, reset next_chunk_size"
              << std::endl;
  }
}

// chunk size: <hex>\r\n
bool HttpRequest::ParseChunkSize(size_t& pos, size_t& chunk_size) {
  chunk_size = 0;
  bool saw_digit = false;
  const size_t k_max_before_shift = std::numeric_limits<size_t>::max() >> 4;
  while (pos < buffer_.size()) {
    char c = buffer_[pos];
    if (c == '\r') break;
    int digit = -1;
    if (c >= '0' && c <= '9') {
      digit = c - '0';
    } else if (c >= 'a' && c <= 'f') {
      digit = c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
      digit = c - 'A' + 10;
    } else {
      // disallow extensions(';' is BAD_REQUEST)
      throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
    }
    if (chunk_size > k_max_before_shift) {
      // overflow
      throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
    }
    chunk_size = (chunk_size << 4) + static_cast<size_t>(digit);
    saw_digit = true;
    ++pos;
  }
  if (!saw_digit) return false;
  return ValidateAndSkipCRLF(pos);
}

// called after reading "0\r\n"; now expect the final CRLF
bool HttpRequest::ValidateFinalCRLF(size_t& pos) {
  std::cerr << "[DEBUG chunk] ValidateFinalCRLF enter"
            << " pos=" << pos << " buffer_size=" << buffer_.size()
            << " remaining=[" << EscapeForDebug(buffer_.substr(pos)) << "]"
            << std::endl;

  if (!ValidateAndSkipCRLF(pos)) return false;
  // ensure no extra data after final CRLF
  if (pos != buffer_.size()) {
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  }
  buffer_.clear();  // erase consumed data including last chunk
  state_ = kDone;
  return true;
}

// append "<data>\r\n" and advance pos, but do not erase yet
bool HttpRequest::AppendChunkData(size_t& pos, size_t chunk_size) {
  // overflow check: body_ size + chunk_size > max_body_size_limit_
  if (chunk_size > max_body_size_limit_ ||
      body_.size() > max_body_size_limit_ - chunk_size) {
    throw lib::exception::ResponseStatusException(lib::http::kPayloadTooLarge);
  }

  const size_t total_needed = chunk_size + 2;
  if (pos + total_needed > buffer_.size()) {
    return false;
  }
  body_.append(buffer_, pos, chunk_size);
  pos += chunk_size;
  return ValidateAndSkipCRLF(pos);
}
