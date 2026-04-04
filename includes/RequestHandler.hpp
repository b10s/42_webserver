#ifndef REQUESTHANDLER_HPP_
#define REQUESTHANDLER_HPP_

#include <dirent.h>    // for DIR, opendir(), readdir(), closedir()
#include <sys/stat.h>  // for struct stat, stat()

#include <algorithm>  // for std::sort
#include <cstdio>     // for std::remove()
#include <fstream>    // for std::ofstream
#include <iostream>   // for debug
#include <sstream>    // for std::stringstream
#include <stdexcept>  // std::runtime_error
#include <vector>

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

class RequestHandler {
 public:
  RequestHandler(ServerConfig conf, HttpRequest req);
  ~RequestHandler();

  ExecResult Run();
  void PrepareRoutingContext();
  std::string ResolveFilesystemPath() const;  // for testing purpose
  std::string AppendIndexFileIfDirectoryOrThrow(
      const std::string &base_path) const;

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
  std::string GenerateDirectoryListing(const std::string &path);
};

#endif
