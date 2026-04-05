#ifndef REQUESTHANDLER_HPP_
#define REQUESTHANDLER_HPP_

#include <dirent.h>    // for DIR, opendir(), readdir(), closedir()
#include <sys/stat.h>  // for struct stat, stat()
#include <cstdio>     // for std::remove()
#include <fstream>    // for std::ofstream
#include <iostream>   // for debug
#include <sstream>    // for std::stringstream
#include <stdexcept>  // std::runtime_error
#include <vector>
#include <ctime>      // for struct tm, localtime_r(), strftime()

#include "ExecResult.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "LocationMatch.hpp"
#include "ServerConfig.hpp"
#include "lib/utils/string_utils.hpp"

struct FileEntry {
  std::string file_name;
  std::string last_modified_time;  // strftime format
  lib::type::Optional<long>
      file_size;  // if directory, file_size.HasValue() == false
  bool is_directory;

  FileEntry(const std::string &file_name, const std::string &last_modified_time,
            lib::type::Optional<long> file_size, bool is_directory);
};

struct GetTarget {
  enum Type { kStaticFile, kCgi, kAutoIndex };

  Type type;
  std::string path;
  std::string body;

  GetTarget() : type(kStaticFile) {
  }
};

class RequestHandler {
 public:
  RequestHandler(ServerConfig conf, HttpRequest req);
  ~RequestHandler();

  ExecResult Run();
  void PrepareRoutingContext();
  std::string ResolveFilesystemPath() const;  // for testing purpose
  std::string AppendIndexFileIfDirectoryOrThrow(
      const std::string &base_path) const;
  GetTarget ResolveGetTarget();  // public for testing purpose

 private:
  RequestHandler();  // shouldn't use default constructor
  ServerConfig conf_;
  HttpRequest req_;
  ExecResult result_;

  LocationMatch location_match_;
  std::string filesystem_path_;
  void HandleGet();
  void HandlePost();
  void HandleDelete();
  std::string GenerateDirectoryListing(const std::string &path,
                                       const std::string &req_uri) const;
};

#endif
