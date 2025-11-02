#include "HttpRequest.hpp"

#include <cstdlib>  // for atoi

// ごく簡単な parseHeader: "Key: Value\r\n" を雑に読む
const char* HttpRequest::parseHeader(const char* req) {
  const char* p = req;

  headers_.clear();

  while (*p) {
    // 空行なら終了（\r\n\r\n）
    if (*p == '\r' && *(p + 1) == '\n') {
      p += 2;
      break;
    }

    // key を読む（':'まで）
    const char* keyStart = p;
    while (*p && *p != ':') ++p;
    if (*p != ':') break;  // 不正でも終了してOK（単純化）

    std::string key(keyStart, p - keyStart);
    ++p;  // skip ':'

    // skip 1つのスペース（あってもなくてもOK）
    if (*p == ' ') ++p;

    // value を読む（\rまで）
    const char* valStart = p;
    while (*p && *p != '\r') ++p;
    std::string val(valStart, p - valStart);

    // CRLF スキップ
    if (*p == '\r' && *(p + 1) == '\n') p += 2;

    // map に追加（小文字化せずそのまま）
    headers_[key] = val;
  }

  // Content-Length ヘッダを確認（単純な整数）
  dict::iterator itCL = headers_.find("Content-Length");
  if (itCL != headers_.end()) {
    contentLength_ = std::atoi(itCL->second.c_str());
  } else {
    contentLength_ = 0;  // 無ければ 0 とみなす
  }

  // Host ヘッダ（あれば記録）
  dict::iterator itH = headers_.find("Host");
  if (itH != headers_.end()) {
    hostName_ = itH->second;
  } else {
    hostName_ = "localhost";
  }

  keepAlive = true;  // とりあえず固定

  return p;
}

// （もしまだ無ければ、ヘッダに追加してOK：bool isDone() const { return progress == DONE; } / const std::string& getBody() const { return body_; }）
// また、例外型とステータスは下記を想定しています：
// http::responseStatusException
// BAD_REQUEST, INTERNAL_SERVER_ERROR（HttpStatus の列挙子）

// std::string HttpRequest::toLowerCopy(const std::string& s) {
//   std::string t = s;
//   for (size_t i = 0; i < t.size(); ++i) t[i] = static_cast<char>(std::tolower(t[i]));
//   return t;
// }

// void HttpRequest::bumpLenOrThrow(size_t& acc, size_t add, size_t limit) {
//   acc += add;
//   if (acc >= limit) {
//     throw http::responseStatusException(REQUEST_HEADER_FIELDS_TOO_LARGE);
//   }
// }

// // 1行を Key: Value\r\n に分解
// //  ":" の直後がスペース1個であること を厳密に要求(※仕様的には OWS＝可変空白が許容だが、簡略化のため)
// // 末尾はCRLF必須
// // req[i+1] アクセス前に ヌル終端チェック を追加して安全化。
// const char* HttpRequest::parseHeaderLine(
//     const char* req, size_t& accLen, std::string& outKey, std::string& outValue)
// {
//   // キーの終端（: まで）
//   size_t i = 0;
//   for (; req[i] && req[i] != ':'; ++i) {
//     bumpLenOrThrow(accLen, 1, kMaxHeaderSize);
//   }
//   if (req[i] != ':' || req[i + 1] == '\0' || req[i + 1] != ' ') {
//     throw http::responseStatusException(BAD_REQUEST);
//   }

//   outKey.assign(req, i);        // [req, req+i)
//   i += 2;                       // ": "
//   bumpLenOrThrow(accLen, 2, kMaxHeaderSize);
//   req += i;                     // 値の先頭へ

//   // 値の終端（\r まで）
//   i = 0;
//   for (; req[i] && req[i] != '\r'; ++i) {
//     bumpLenOrThrow(accLen, 1, kMaxHeaderSize);
//   }
//   if (req[i] != '\r' || req[i + 1] == '\0' || req[i + 1] != '\n') {
//     throw http::responseStatusException(BAD_REQUEST);
//   }

//   if (i == 0) outValue.clear();
//   else        outValue.assign(req, i);

//   req += i + 2;                 // "\r\n" を消費
//   bumpLenOrThrow(accLen, 2, kMaxHeaderSize);
//   return req;
// }

// // parseHostHeader は hostName_ / hostPort_ を設定する。
// void HttpRequest::parseHostHeader(const std::string& host) {
//   size_t i = 0;
//   while (i < host.size() && host[i] != ':') ++i;
//   if (i == 0) throw http::responseStatusException(BAD_REQUEST); // 空のホスト名

//   hostName_ = host.substr(0, i);
//   if (i == host.size()) {
//     hostPort_ = DEFAULT_PORT;          // 例: "8080"
//   } else {
//     hostPort_ = host.substr(i + 1);    // ポート文字列（数字チェック等は必要に応じて追加）
//   }
// }

// void HttpRequest::validateAndApplyHeaders() {
//   // Host (キーは小文字保存方針なので "host")
//   dict::const_iterator itHost = headers_.find("host");
//   if (itHost == headers_.end()) {
//     throw http::responseStatusException(BAD_REQUEST);
//   }
//   parseHostHeader(itHost->second);

//   const bool hasCL  = headers_.find("content-length")   != headers_.end();
//   const bool hasTE  = headers_.find("transfer-encoding")!= headers_.end();

//   if (hasCL && hasTE) {
//     throw http::responseStatusException(BAD_REQUEST);
//   } else if (!hasCL && !hasTE) {
//     if (method_ == POST || method_ == PUT) {
//       throw http::responseStatusException(LENGTH_REQUIRED);
//     }
//   }

//   if (hasCL) {
//     std::stringstream ss(headers_["content-length"]);
//     ss >> contentLength_;
//     if (ss.fail() || contentLength_ < 0 ||
//         static_cast<size_t>(contentLength_) > kMaxPayloadSize) {
//       throw http::responseStatusException(BAD_REQUEST);
//     }
//   } else if (hasTE) {
//     // 値は大小無視で比較（保存時点でlower済）
//     if (headers_["transfer-encoding"] == "chunked") contentLength_ = -1;
//     else throw http::responseStatusException(NOT_IMPLEMENTED);
//   } else {
//     contentLength_ = 0;
//   }

//   dict::const_iterator itConn = headers_.find("connection");
//   if (itConn != headers_.end()) {
//     const std::string& v = itConn->second; // すでにlower
//     if      (v == "close")      keepAlive = false;
//     else if (v == "keep-alive") keepAlive = true;
//     else                        throw http::responseStatusException(BAD_REQUEST);
//   }
// }

// const char* HttpRequest::parseHeader(const char* req)
// {
//   size_t acc = 0;
//   while (*req && req[0] != '\r') {
//     std::string key, val;
//     req = parseHeaderLine(req, acc, key, val);

//     // キーを小文字化して保存（Host/host/HOST を同一視）
//     key = toLowerCopy(key);
//     headers_[key] = val; // 重複キーが来た場合の方針は必要に応じて（上書き/検証）
//   }

//   // 空行の存在チェック
//   if (req[0] != '\r' || req[1] == '\0' || req[1] != '\n') {
//     throw http::responseStatusException(BAD_REQUEST);
//   }

//   // ここまでで headers_ が埋まったので、まとめて検証＆クラス状態へ反映
//   validateAndApplyHeaders();

//   return req; // 呼び出し側で "\r\n" をさらにスキップしてボディへ
// }


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// // ヘッダー1行は基本 Key: Value\r\n
// //
// const char *HttpRequest::parseHeader(const char *req)
// {
//   size_t len = 0;
//   while (*req && req[0] != '\r')
//   {
//     size_t i = 0;
//     for (; req[i] && req[i] != ':'; i++)
//     {
//       if (++len >= kMaxHeaderSize)
//       {
//         throw http::responseStatusException(REQUEST_HEADER_FIELDS_TOO_LARGE);
//       }
//     }
//     if (req[i] != ':' || req[i + 1] != ' ')
//     {
//       throw http::responseStatusException(BAD_REQUEST);
//     }
//     std::string key = std::string(req, i); // [req, req+i) をキーに対応する値としてパース
//     i += 2; // Skip ": "
//     len += 2;
//     if (len >= kMaxHeaderSize)
//     {
//       throw http::responseStatusException(REQUEST_HEADER_FIELDS_TOO_LARGE);
//     }
//     req += i; // 行ポインタを値の先頭へ
//     i = 0;
//     for (; req[i] && req[i] != '\r'; i++) // 値の終端 '\r' まで
//     {
//       if (++len >= kMaxHeaderSize)
//       {
//         throw http::responseStatusException(REQUEST_HEADER_FIELDS_TOO_LARGE);
//       }
//     }
//     if (req[i] != '\r' || req[i + 1] != '\n') // req が指す文字列（受信バッファ）の先頭から、空行（\r\n）に出会うまでの「ヘッダー部」を1行ずつパース
//     {
//       throw http::responseStatusException(BAD_REQUEST);
//     }
//     if (i == 0)
//     {
//       this->headers_[key] = ""; // allow empty value
//     }
//     else
//     {
//       this->headers_[key] = std::string(req, i);
//     }
//     req += i + 2; // Skip "\r\n"
//     len += 2;
//     if (len >= kMaxHeaderSize) // avoid DoS by too large headers
//     {
//       throw http::responseStatusException(REQUEST_HEADER_FIELDS_TOO_LARGE);
//     }
//   }
//   // validation
//   // 必須・矛盾チェック
//   // Host、Content-Length/Transfer-Encoding 等）を行い、hostName_ / hostPort_ / contentLength_ / keepAlive を設定する
//   if (this->headers_.find("Host") == this->headers_.end())
//   {
//     throw http::responseStatusException(BAD_REQUEST);
//   }
//   else
//   {
//     std::string host = this->headers_["Host"]; // Host/host混在に注意　小文字に統一する必要があるかも？もしくはどちらかを文法エラーにする？
//     size_t i = 0;
//     while (i < host.size() && host[i] != ':')
//     {
//       i++;
//     }
//     if (i == 0) // empty hostname
//     {
//       throw http::responseStatusException(BAD_REQUEST); // 400
//     }
//     this->hostName_ = host.substr(0, i);
//     if (i == host.size()) // no port specified
//     {
//       this->hostPort_ = DEFAULT_PORT; // DEFAULT_PORT "8080"
//     }
//     else
//     {
//       this->hostPort_ = host.substr(i + 1);
//     }
//   }
//   bool hasContentLength = headers_.find("Content-Length") != headers_.end();
//   bool hasTransferEncoding = headers_.find("Transfer-Encoding") != headers_.end();
//   if (hasContentLength && hasTransferEncoding) // RFC allows both, but we disallow both for simplicity
//     throw http::responseStatusException(BAD_REQUEST);
//   }
//   else if (!hasContentLength && !hasTransferEncoding)
//   {
//     if (method_ == POST) // no content-length and transfar-encoding, we don't know body length
//     {
//       throw http::responseStatusException(LENGTH_REQUIRED);
//     }
//   }
//   // set content length
//   if (hasContentLength)
//   {
//     std::stringstream ss(headers_["Content-Length"]); // parse to long
//     ss >> contentLength_;
//     if (ss.fail() || contentLength_ < 0 || static_cast<size_t>(contentLength_) > kMaxPayloadSize)
//     {
//       throw http::responseStatusException(BAD_REQUEST); // invalid content-length
//     }
//   }
//   else if (hasTransferEncoding)
//   {
//     if (headers_["Transfer-Encoding"] == "chunked")
//     {
//       contentLength_ = -1; // chunked transfer encoding
//     } else {
//       throw http::responseStatusException(NOT_IMPLEMENTED);
//     }
//   }
//   else
//   {
//     contentLength_ = 0;
//   }
//   bool hasConnection = headers_.find("Connection") != headers_.end();
//   if (hasConnection)
//   {
//     if (headers_["Connection"] == "close")
//       keepAlive = false;
//     else if (headers_["Connection"] == "keep-alive")
//       keepAlive = true;
//     else
//       throw http::responseStatusException(BAD_REQUEST); // we allow only "close" or "keep-alive", any other value is invalid including "Close" or "KEEP-ALIVE"
//   }
//   return req;
// }

// 返すのは空行の先頭（\r\n\r\n の 前半 \r を指す位置）。
// 呼び出し側は さらに \r\n を 1 回スキップしてから、ボディのパースへ進みます。
// もし「戻り値をボディ先頭にしたい」なら、ここで req += 2;（空行の \r\n）を進めてから返す設計にしてもOK。
