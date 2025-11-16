#ifndef LOCATION_HPP_
#define LOCATION_HPP_

#include <set>
#include <string>
#include <vector>

#include "enums.hpp"

class Location {
 private:
  // parsed
  std::set<RequestMethod> methods_;
  std::string name_;
  std::string root_;
  bool autoindex_;
  std::vector<std::string> index_files_;
  std::string extensions_;  // we are not doing bonus so only one extension is
                            // allowed here
  std::string upload_path_;
  std::string redirect_;
  std::string cgi_path_;

 public:
  Location();

  void SetName(const std::string& name) {
    name_ = name;
  }

  const std::string& GetName() const {
    return name_;
  }

  void SetRoot(const std::string& root) {
    root_ = root;
  }

  const std::string& GetRoot() const {
    return root_;
  }

  void SetAutoIndex(bool autoindex) {
    autoindex_ = autoindex;
  }

  bool GetAutoIndex() const {
    return autoindex_;
  }

  void AddIndex(const std::string& index) {
    index_files_.push_back(index);
  }

  const std::vector<std::string>& GetIndexFiles() const {
    return index_files_;
  }

  void SetExtension(const std::string& ext) {
    extensions_ = ext;
  }

  const std::string& GetExtensions() const {
    return extensions_;
  }

  void SetUploadPath(const std::string& path) {
    upload_path_ = path;
  }

  const std::string& GetUploadPath() const {
    return upload_path_;
  }

  void SetRedirect(const std::string& redirect) {
    redirect_ = redirect;
  }

  const std::string& GetRedirect() const {
    return redirect_;
  }

  void SetCgiPath(const std::string& path) {
    cgi_path_ = path;
  }

  const std::string& GetCgiPath() const {
    return cgi_path_;
  }

  void AddMethod(RequestMethod method) {
    methods_.insert(method);
  }

  bool IsMethodAllowed(RequestMethod method) const {
    return methods_.count(method) > 0;
  }

  std::set<RequestMethod> GetMethods() const {
    return methods_;
  }
};

#endif  // LOCATION_HPP_
