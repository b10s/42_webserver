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
  std::vector<std::string> indexFiles_;
  std::string extensions_;  // we are not doing bonus so only one extension is
                            // allowed here
  std::string uploadPath_;
  std::string redirect_;
  std::string cgiPath_;

 public:
  Location();
  void setName(const std::string& name) { name_ = name; }
  const std::string& getName() const { return name_; }
  void setRoot(const std::string& root) { root_ = root; }
  const std::string& getRoot() const { return root_; }
  void setAutoIndex(bool autoindex) { autoindex_ = autoindex; }
  bool getAutoIndex() const { return autoindex_; }
  void addIndex(const std::string& index) { indexFiles_.push_back(index); }
  const std::vector<std::string>& getIndexFiles() const { return indexFiles_; }
  void setExtension(const std::string& ext) { extensions_ = ext; }
  const std::string& getExtensions() const { return extensions_; }
  void setUploadPath(const std::string& path) { uploadPath_ = path; }
  const std::string& getUploadPath() const { return uploadPath_; }
  void setRedirect(const std::string& redirect) { redirect_ = redirect; }
  const std::string& getRedirect() const { return redirect_; }
  void setCgiPath(const std::string& path) { cgiPath_ = path; }
  const std::string& getCgiPath() const { return cgiPath_; }
  void addMethod(RequestMethod method) { methods_.insert(method); }
  bool isMethodAllowed(RequestMethod method) const {
    return methods_.count(method) > 0;
  }
  std::set<RequestMethod> getMethods() const { return methods_; }
};

#endif  // LOCATION_HPP_
