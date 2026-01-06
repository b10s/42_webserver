#include "lib/parser/StreamParser.hpp"

#include <string>

#include "lib/exception/ResponseStatusException.hpp"
#include "lib/http/CharValidation.hpp"
#include "lib/http/Status.hpp"

namespace lib {
namespace parser {

StreamParser::StreamParser() : buffer_read_pos_(0), state_(kHeader) {
}

StreamParser::~StreamParser() {
}

void StreamParser::Parse(const char* data, size_t len) {
  buffer_.append(data, len);
  for (;;) {
    switch (state_) {
      case kHeader:
        if (!AdvanceHeader()) return;
        if (state_ != kBody) OnInternalStateError();
        continue;

      case kBody:
        if (!AdvanceBody()) return;
        if (state_ != kDone) OnInternalStateError();
        return;

      case kDone:
        OnExtraDataAfterDone();
    }
  }
}

bool StreamParser::IsCRLF(const char* p) const {
  return p != NULL && p[0] == '\r' && p[1] == '\n';
}

bool StreamParser::IsLF(const char* p) const {
  return p != NULL && p[0] == '\n';
}

std::string::size_type StreamParser::FindEndOfHeader(
    const std::string& payload) {
  size_t pos = payload.find("\r\n\r\n");
  if (pos != std::string::npos) return pos + 4;
  if (!IsStrictCrlf()) {
    pos = payload.find("\n\n");
    if (pos != std::string::npos) return pos + 2;
  }
  return std::string::npos;
}

void StreamParser::BumpLenOrThrow(size_t& total, size_t inc,
                                  size_t max_size) const {
  if (inc > max_size - total) {
    throw lib::exception::ResponseStatusException(
        lib::http::kRequestHeaderFieldsTooLarge);
  }
  total += inc;
}

const char* StreamParser::ReadHeaderLine(const char* data, std::string& key,
                                         std::string& value, size_t& total_len,
                                         size_t max_size) {
  size_t i = 0;
  while (data[i] && data[i] != ':') {
    if (!lib::http::IsValidHeaderChar(data[i])) {
      throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
    }
    BumpLenOrThrow(total_len, 1, max_size);
    ++i;
  }
  // must be ": ", not ":" or end of string
  if (data[i] != ':' || req[i + 1] == '\0' || data[i + 1] != ' ') {
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  }
  key.assign(data, i);
  // skip": "
  i += 2;
  BumpLenOrThrow(total_len, 2, max_size);
  data += i;
  size_t vlen = 0;
  while (data[vlen] && data[vlen] != '\r' && data[vlen] != '\n') {
    if (!lib::http::IsValidHeaderChar(data[vlen])) {
      throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
    }
    BumpLenOrThrow(total_len, 1, max_size);
    ++vlen;
  }
  value.assign(data, vlen);  // value can be empty
  if (IsCRLF(data + vlen)) {
    BumpLenOrThrow(total_len, 2, max_size);  // skip CRLF
    return data + vlen + 2;
  } else if (!IsStrictCrlf() && IsLF(data + vlen)) {
    BumpLenOrThrow(total_len, 1, max_size);  // skip LF
    return data + vlen + 1;
  }
  throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
}

}  // namespace parser
}  // namespace lib
