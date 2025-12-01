#ifndef LOCATION_HPP_
#define LOCATION_HPP_

#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "lib/http/Method.hpp"

class Location {
 private:
  // parsed
  std::set<lib::http::Method> methods_;  // methods can be multiple
  std::string name_;
  std::string root_;
  bool autoindex_;
  std::vector<std::string> index_files_;  // multiple index files allowed
  std::string extensions_;
  std::string upload_path_;
  std::string redirect_;
  std::string cgi_path_;
  bool has_allow_methods_;  // method directive should appear only once
  bool has_root_;
  bool has_autoindex_;
  bool has_index_directive_;  // index directive should appear only once
  bool has_extensions_;
  bool has_upload_path_;
  bool has_redirect_;
  bool has_cgi_path_;

 public:
  Location();

  void AddMethod(lib::http::Method method) {
    methods_.insert(method);
  }

  bool IsMethodAllowed(lib::http::Method method) const {
    return methods_.count(method) > 0;
  }

  std::set<lib::http::Method> GetMethods() const {
    return methods_;
  }

  bool HasAllowMethods() const {
    return has_allow_methods_;
  }

  void SetHasAllowMethods(bool has) {
    has_allow_methods_ = has;
  }

  void SetName(const std::string& name) {
    name_ = name;
  }

  const std::string& GetName() const {
    return name_;
  }

  void SetRoot(const std::string& root) {
    if (has_root_) {
      throw std::runtime_error("Duplicate root directive");
    }
    root_ = root;
    has_root_ = true;
  }

  const std::string& GetRoot() const {
    return root_;
  }

  void SetAutoIndex(bool autoindex) {
    if (has_autoindex_) {
      throw std::runtime_error("Duplicate autoindex directive");
    }
    autoindex_ = autoindex;
    has_autoindex_ = true;
  }

  bool GetAutoIndex() const {
    return autoindex_;
  }

  void AddIndex(const std::string& index) {
    index_files_.push_back(index);
  }

  bool HasIndexDirective() const {
    return has_index_directive_;
  }

  void SetHasIndexDirective(bool has) {
    has_index_directive_ = has;
  }

  const std::vector<std::string>& GetIndexFiles() const {
    return index_files_;
  }

  void SetExtension(const std::string& ext) {
    if (has_extensions_) {
      throw std::runtime_error("Duplicate extension directive");
    }
    extensions_ = ext;
    has_extensions_ = true;
  }

  const std::string& GetExtensions() const {
    return extensions_;
  }

  void SetUploadPath(const std::string& path) {
    if (has_upload_path_) {
      throw std::runtime_error("Duplicate upload_path directive");
    }
    upload_path_ = path;
    has_upload_path_ = true;
  }

  const std::string& GetUploadPath() const {
    return upload_path_;
  }

  void SetRedirect(const std::string& redirect) {
    if (has_redirect_) {
      throw std::runtime_error("Duplicate redirect directive");
    }
    redirect_ = redirect;
    has_redirect_ = true;
  }

  const std::string& GetRedirect() const {
    return redirect_;
  }

  void SetCgiPath(const std::string& path) {
    if (has_cgi_path_) {
      throw std::runtime_error("Duplicate cgi_path directive");
    }
    cgi_path_ = path;
    has_cgi_path_ = true;
  }

  const std::string& GetCgiPath() const {
    return cgi_path_;
  }
};

#endif  // LOCATION_HPP_
