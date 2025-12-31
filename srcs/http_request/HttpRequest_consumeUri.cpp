#include "HttpRequest.hpp"
#include "lib/exception/ResponseStatusException.hpp"
#include "lib/http/CharValidation.hpp"
#include "lib/http/Status.hpp"

inline void BumpOrThrow(std::size_t& len) {
  if (len >= HttpRequest::kMaxUriSize) {
    throw lib::exception::ResponseStatusException(lib::http::kUriTooLong);
  }
}

// simple check for origin-form path and query
// we are not implementing pct-encoding (%HH) here for simplicity
// query = *( pchar / "/" / "?" )  ... "/" and "?" are allowed in query
inline const char* ConsumeUntilStopChar(const char* req, std::size_t& len,
                                        const char* stop_char) {
  while (*req && std::strchr(stop_char, *req) == 0) {
    if (*req == '#') {  // fragment not allowed in HTTP request URI
      throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
    }
    ++len;
    BumpOrThrow(len);
    if (!lib::http::IsVisibleAscii(*req))
      throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
    ++req;
  }
  return req;
}

// read query--- req points to the next char after '?'
// len is the length of the URI part read so far (including '?')
const char* HttpRequest::ConsumeQuery(const char* req, std::size_t& len) {
  while (true) {
    const char* key_begin = req;
    const char* key_end = ConsumeUntilStopChar(req, len, " =&");
    std::string key(key_begin, key_end - key_begin);  // key should not be empty
    req = key_end;
    if (key.empty())
      throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
    std::string val;  // val can be empty so initialize as empty
    if (*req == '=') {
      ++req;  // skip '='
      ++len;  // add '=' to length
      BumpOrThrow(len);
      const char* val_begin = req;
      const char* val_end = ConsumeUntilStopChar(req, len, " &");
      val.assign(val_begin, val_end - val_begin);
      req = val_end;
    }
    query_[key] = val;
    // next pair or end
    if (*req == '&') {
      ++req;
      ++len;  // add '&' to length
      BumpOrThrow(len);
    } else {
      break;
    }
  }
  return req;
}

const char* HttpRequest::ConsumeUri(const char* req) {
  std::size_t len = 0;
  if (*req != '/') {  // origin-form must start with '/'
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  }

  const char* path_stop = ConsumeUntilStopChar(req, len, " ?");
  uri_.assign(req, path_stop - req);
  if (uri_.empty() || (*path_stop != ' ' && *path_stop != '?')) {
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  }
  req = path_stop;
  if (*req == '?') {
    ++req;  // skip '?'
    ++len;  // add '?' to length
    BumpOrThrow(len);
    req = ConsumeQuery(req, len);
  }
  if (*req != ' ') {
    throw lib::exception::ResponseStatusException(lib::http::kBadRequest);
  }
  return req + 1;  // skip space after URI and move to consume version
}
