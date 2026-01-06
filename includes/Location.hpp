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
  std::string index_file_;
  std::string upload_path_;
  std::string redirect_;
  bool cgi_enabled_;
  std::vector<std::string> cgi_allowed_extensions_;
  bool has_allow_methods_;  // method directive should appear only once
  bool has_root_;
  bool has_autoindex_;
  bool has_index_directive_;  // index directive should appear only once
  bool has_upload_path_;
  bool has_redirect_;
  bool has_cgi_enabled_;
  bool has_cgi_allowed_extensions_;

 public:
  Location();

  void AddCgiAllowedExtension(const std::string& ext) {
    cgi_allowed_extensions_.push_back(ext);
  }

  void SetHasCgiAllowedExtensions(bool has) {
    has_cgi_allowed_extensions_ = has;
  }

  const std::vector<std::string>& GetCgiAllowedExtensions() const {
    return cgi_allowed_extensions_;
  }

  bool HasCgiAllowedExtensions() const {
    return has_cgi_allowed_extensions_;
  }

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

  void SetIndexFile(const std::string& index) {
    if (has_index_directive_) {
      throw std::runtime_error("Duplicate index directive");
    }
    index_file_ = index;
    has_index_directive_ = true;
  }

  const std::string& GetIndexFile() const {
    return index_file_;
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

  void SetCgiEnabled(const std::string& value) {
    if (has_cgi_enabled_) {
      throw std::runtime_error("Duplicate cgi directive");
    }
    if (value != "on" && value != "off") {
      throw std::runtime_error("Invalid cgi value: " + value);
    }
    cgi_enabled_ = (value == "on");
    has_cgi_enabled_ = true;
  }

  bool GetCgiEnabled() const {
    return cgi_enabled_;
  }
};

#endif  // LOCATION_HPP_
