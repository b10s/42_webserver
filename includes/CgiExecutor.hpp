#ifndef CGIEXECUTOR_HPP_
#define CGIEXECUTOR_HPP_

#include <unistd.h>

#include <map>
#include <string>
#include <vector>

#include "ExecResult.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Location.hpp"
#include "lib/type/Optional.hpp"

class CgiExecutor {
 private:
  // Prohibit to use default constructor
  CgiExecutor();

  enum { kReadEnd = 0, kWriteEnd = 1 };

  std::map<std::string, lib::type::Optional<std::string> > meta_vars_;
  std::string GetMetaVar(const std::string&) const;
  std::vector<std::string> GetMetaVars() const;
  void InitializeMetaVars(const HttpRequest&);

  const Location& loc_;
  std::string script_path_;
  std::string body_;

 public:
  CgiExecutor(const HttpRequest&, const Location&, const std::string&);
  ~CgiExecutor();

  ExecResult Run();
};

#endif
