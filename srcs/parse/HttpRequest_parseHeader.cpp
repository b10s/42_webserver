#include "HttpRequest.hpp"

const char *HttpRequest::parseHeader(const char *req)
{
  size_t len = 0;
  while (*req && req[0] != '\r')
  {
    size_t i = 0;
    for (; req[i] && req[i] != ':'; i++)
    {
      if (++len >= kMaxHeaderSize)
      {
        throw http::responseStatusException(REQUEST_HEADER_FIELDS_TOO_LARGE);
      }
    }
    if (req[i] != ':' || req[i + 1] != ' ')
    {
      throw http::responseStatusException(BAD_REQUEST);
    }
    std::string key = std::string(req, i);
    i += 2; // Skip ": "
    len += 2;
    if (len >= kMaxHeaderSize)
    {
      throw http::responseStatusException(REQUEST_HEADER_FIELDS_TOO_LARGE);
    }
    req += i;
    i = 0;
    for (; req[i] && req[i] != '\r'; i++)
    {
      if (++len >= kMaxHeaderSize)
      {
        throw http::responseStatusException(REQUEST_HEADER_FIELDS_TOO_LARGE);
      }
    }
    if (req[i] != '\r' || req[i + 1] != '\n')
    {
      throw http::responseStatusException(BAD_REQUEST);
    }
    if (i == 0)
    {
      this->headers_[key] = "";
    }
    else
    {
      this->headers_[key] = std::string(req, i);
    }
    req += i + 2; // Skip "\r\n"
    len += 2;
    if (len >= kMaxHeaderSize)
    {
      throw http::responseStatusException(REQUEST_HEADER_FIELDS_TOO_LARGE);
    }
  }
  // validation
  if (this->headers_.find("Host") == this->headers_.end())
  {
    throw http::responseStatusException(BAD_REQUEST);
  }
  else
  {
    std::string host = this->headers_["Host"];
    size_t i = 0;
    while (i < host.size() && host[i] != ':')
    {
      i++;
    }
    if (i == 0)
    {
      throw http::responseStatusException(BAD_REQUEST);
    }
    this->hostName_ = host.substr(0, i);
    if (i == host.size())
    {
      this->hostPort_ = DEFAULT_PORT;
    }
    else
    {
      this->hostPort_ = host.substr(i + 1);
    }
  }
  bool hasContentLength = headers_.find("Content-Length") != headers_.end();
  bool hasTransferEncoding = headers_.find("Transfer-Encoding") != headers_.end();
  if (hasContentLength && hasTransferEncoding)
  {
    throw http::responseStatusException(BAD_REQUEST);
  }
  else if (!hasContentLength && !hasTransferEncoding)
  {
    if (method_ == POST || method_ == PUT)
    {
      throw http::responseStatusException(LENGTH_REQUIRED);
    }
  }
  // set content length
  if (hasContentLength)
  {
    std::stringstream ss(headers_["Content-Length"]);
    ss >> contentLength_;
    if (ss.fail() || contentLength_ < 0 || static_cast<size_t>(contentLength_) > kMaxPayloadSize)
    {
      throw http::responseStatusException(BAD_REQUEST);
    }
  }
  else if (hasTransferEncoding)
  {
    if (headers_["Transfer-Encoding"] == "chunked")
    {
      contentLength_ = -1;
    } else {
      throw http::responseStatusException(NOT_IMPLEMENTED);
    }
  }
  else
  {
    contentLength_ = 0;
  }
  bool hasConnection = headers_.find("Connection") != headers_.end();
  if (hasConnection)
  {
    if (headers_["Connection"] == "close")
      keepAlive = false;
    else if (headers_["Connection"] == "keep-alive")
      keepAlive = true;
    else
      throw http::responseStatusException(BAD_REQUEST);
  }
  return req;
}
