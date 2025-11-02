#include "HttpRequest.hpp"

// --- 最小 parseUri: 次のスペースまでを URI として採用（上限チェックなし） ---
const char* HttpRequest::parseUri(const char* req) {
  const char* p = req;
  while (*p && *p != ' ' && *p != '\r' && *p != '\n') ++p;
  if (*p != ' ') throw http::responseStatusException(BAD_REQUEST);
  uri_.assign(req, p - req);
  return p + 1; // スペースを飛ばす
}

// namespace {
// inline bool isVisibleAscii(char c) {
//   return c >= '!' && c <= '~';
// }
// }

// inline void bumpOrThrow(std::size_t& len, std::size_t limit) {
//   if (++len >= limit) {
//     throw http::responseStatusException(URI_TOO_LONG);
//   }
// }

// const char *HttpRequest::parseUri(const char *req)
// {
//   size_t len = 0;
//   std::size_t i = 0;
//   for (; req[i] != ' ' && req[i] != '?'; i++)
//   {
//     bumpOrThrow(len, kMaxUriSize);
//     if (!isVisibleAscii(req[i]))
//       throw http::responseStatusException(BAD_REQUEST);
//   }
//   uri_.assign(req, i);
//   if (this->uri_.empty() || (req[i] != ' ' && req[i] != '?'))
//   {
//     throw http::responseStatusException(BAD_REQUEST);
//   }


//   // req += i;
//   // // Parse query
//   // if (*req == '?')
//   // {
//   //   req++;
//   //   if (++len >= kMaxUriSize)
//   //   {
//   //     throw http::responseStatusException(URI_TOO_LONG);
//   //   }
//   //   while (true)
//   //   {
//   //     // Parse key
//   //     std::string key;
//   //     i = 0;
//   //     for (; req[i] && (req[i] != '&' && req[i] != '=' && req[i] != ' '); i++)
//   //     {
//   //       if (++len >= kMaxUriSize)
//   //       {
//   //         throw http::responseStatusException(URI_TOO_LONG);
//   //       }
//   //       if (req[i] < '!' || req[i] > '~')
//   //       {
//   //         throw http::responseStatusException(BAD_REQUEST);
//   //       }
//   //     }
//   //     key = std::string(req, i);
//   //     // Parse value
//   //     if (req[i] == '=')
//   //     {
//   //       req += i + 1;
//   //       i = 0;
//   //       for (; req[i] && (req[i] != '&' && req[i] != ' '); i++)
//   //       {
//   //         if (++len >= kMaxUriSize)
//   //         {
//   //           throw http::responseStatusException(URI_TOO_LONG);
//   //         }
//   //         if (req[i] < '!' || req[i] > '~')
//   //         {
//   //           throw http::responseStatusException(BAD_REQUEST);
//   //         }
//   //       }
//   //       this->query_[key] = std::string(req, i);
//   //     }
//   //     else
//   //     {
//   //       this->query_[key] = "";
//   //     }
//   //     req += i;
//   //     if (*req == '&')
//   //     {
//   //       if (++len >= kMaxUriSize)
//   //       {
//   //         throw http::responseStatusException(URI_TOO_LONG);
//   //       }
//   //       req++;
//   //     }
//   //     else
//   //     {
//   //       break;
//   //     }
//   //   }
//   // }
//   // if (*req != ' ')
//   // {
//   //   throw http::responseStatusException(BAD_REQUEST);
//   // }
//   return req + 1;
// }

