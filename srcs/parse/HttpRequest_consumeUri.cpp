#include "HttpRequest.hpp"

namespace {
inline bool isVisibleAscii(char c) {
  return c >= '!' && c <= '~';
}

inline void bumpOrThrow(std::size_t& len) {
  if (len >= HttpRequest::kMaxUriSize) {
    throw http::responseStatusException(URI_TOO_LONG);
  }
}

// simple check for origin-form path and query
// we are not implementing pct-encoding (%HH) here for simplicity
// query = *( pchar / "/" / "?" )  ... "/" and "?" are allowed in query
inline const char* consumeUntilStopChar(const char* req, std::size_t& len,
                                        const char* stopChar) {
  while (*req && std::strchr(stopChar, *req) == 0) {
    if (*req == '#') {  // fragment not allowed in HTTP request URI
      throw http::responseStatusException(BAD_REQUEST);
    }
    ++len;
    bumpOrThrow(len);
    if (!isVisibleAscii(*req)) throw http::responseStatusException(BAD_REQUEST);
    ++req;
  }
  return req;
}
}  // namespace

// read query--- req points to the next char after '?'
// len is the length of the URI part read so far (including '?')
const char* HttpRequest::consumeQuery(const char* req, std::size_t& len) {
  while (true) {
    const char* key_begin = req;
    const char* key_end = consumeUntilStopChar(req, len, " =&");
    std::string key(key_begin, key_end - key_begin);  // key should not be empty
    req = key_end;
    if (key.empty()) throw http::responseStatusException(BAD_REQUEST);
    std::string val;  // val can be empty so initialize as empty
    if (*req == '=') {
      ++req;  // skip '='
      ++len;  // add '=' to length
      bumpOrThrow(len);
      const char* val_begin = req;
      const char* val_end = consumeUntilStopChar(req, len, " &");
      val.assign(val_begin, val_end - val_begin);
      req = val_end;
    }
    query_[key] = val;
    // next pair or end
    if (*req == '&') {
      ++req;
      ++len;  // add '&' to length
      bumpOrThrow(len);
    } else {
      break;
    }
  }
  return req;
}

const char* HttpRequest::consumeUri(const char* req) {
  std::size_t len = 0;
  if (*req != '/') {  // origin-form must start with '/'
    throw http::responseStatusException(BAD_REQUEST);
  }

  const char* path_stop = consumeUntilStopChar(req, len, " ?");
  uri_.assign(req, path_stop - req);
  if (uri_.empty() || (*path_stop != ' ' && *path_stop != '?')) {
    throw http::responseStatusException(BAD_REQUEST);
  }
  req = path_stop;
  if (*req == '?') {
    ++req;  // skip '?'
    ++len;  // add '?' to length
    bumpOrThrow(len);
    req = consumeQuery(req, len);
  }
  if (*req != ' ') {
    throw http::responseStatusException(BAD_REQUEST);
  }
  return req + 1;  // skip space after URI and move to consume version
}
