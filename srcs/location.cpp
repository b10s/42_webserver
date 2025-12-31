#include "Location.hpp"

Location::Location()
    : methods_(),
      name_("/"),
      root_("./"),
      autoindex_(false),
      index_file_(),
      upload_path_(),
      redirect_(),
      cgi_enabled_(false),
      cgi_allowed_extensions_(),
      has_allow_methods_(false),
      has_root_(false),
      has_autoindex_(false),
      has_index_directive_(false),
      has_upload_path_(false),
      has_redirect_(false),
      has_cgi_enabled_(false),
      has_cgi_allowed_extensions_(false) {
}
