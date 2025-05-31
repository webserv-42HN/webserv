


#include "../includes/Response.hpp"
#include "../includes/Router.hpp"
#include "../includes/Request.hpp"
#include <limits>

Response::Response(std::vector<ServerConfig> config): Rconfig(config) {}

Response::~Response() {}

bool Response::isCGIRequest(const std::string& url) {
  // First check if it's in the CGI directory
  bool in_cgi_dir = url.find("/cgi/") != std::string::npos;
  
  // Then check if it has a script extension
  bool has_script_ext = url.find(".cgi") != std::string::npos ||
                        url.find(".py") != std::string::npos ||
                        url.find(".php") != std::string::npos;
  
  // If in CGI directory, only treat as CGI if it has a script extension
  if (in_cgi_dir) {
    return has_script_ext;
  }
  
  // Otherwise, just check extension
  return has_script_ext;
}

std::string Response::routing(std::string method, std::string url) {
    Router router(Rconfig);
    // Check if it's a CGI request before adding a trailing slash
    bool is_cgi = isCGIRequest(url);

    // Only add trailing slash for non-CGI URLs that don't already have one
    if (!is_cgi && !url.empty() && url.back() != '/') {
        url += '/';
    }
    t_routeConfig config = router.getRouteConfig(url);
    std::string full_path = config.root_dir + url;

    if (is_cgi) {
      // First check if the path is a directory
      if (isDirectory(full_path)) {
          // Handle CGI directory similar to regular directories
          if (!config.default_file.empty()) {
              std::string index_path = full_path + config.default_file;
              std::cout << "DEBUG: CGI INDEX PATH: " << index_path << std::endl;
              
              // Check if the default file exists
              std::ifstream index_file(index_path);
              if (index_file.good()) {
                  index_file.close();
                  // Extract query string if present
                  std::string query_string = "";
                  size_t query_pos = url.find('?');
                  if (query_pos != std::string::npos) {
                      query_string = url.substr(query_pos + 1);
                      url = url.substr(0, query_pos);
                  }
                  // Execute the default file as CGI
                  return executeCGI(index_path, query_string, method);
              }
          }
          
          // If no default file or it doesn't exist, show directory listing or 404
          if (config.autoindex) {
              return generateDirectoryListing(full_path, url);
          }
          return getErrorResponse(404); // No default file and autoindex is off
      }
      
      // Not a directory, process as normal CGI
      std::string query_string = "";
      size_t query_pos = url.find('?');
      if (query_pos != std::string::npos) {
          query_string = url.substr(query_pos + 1);
          url = url.substr(0, query_pos);
      }
      return executeCGI(full_path, query_string, method);
  }
    if (isDirectory(full_path) && method == "GET") {
        // Check if default_file is specified
        if (!config.default_file.empty()) {
            std::string index_path = full_path + config.default_file;
            std::cout << "DEBUG: INDEX PATH: " << index_path << std::endl;
            std::ifstream index_file(index_path);
            if (index_file.good()) {
                index_file.close();
                return getGetResponse(index_path, 200);
            }
        }
        if (config.autoindex) {
            return generateDirectoryListing(full_path, url);
        }
        return getErrorResponse(404); // No default file and autoindex is off
    }
    if (full_path.back() == '/')
        full_path.pop_back(); // Remove trailing slash for file access
    return generatingResponse(methodToEnum(method), full_path);
}

std::string Response::generatingResponse(HttpMethod method, std::string full_url) {
    std::string response;
    switch (method)
    {
    case GET:
        response = getGetResponse(full_url, 200);
        break;
    case POST:
        response = getPostResponse(full_url);
        break;
    case DELETE:
        response = getDeleteResponse(full_url);
        break;
    case HEAD:
        response = getHeadResponse(full_url, 200);
        break;
    case UNKNOWN:
        response = getErrorResponse(405);
        break;
    default:
        break;
    }
    return response;
}

std::string Response::getGetResponse(const std::string& requested_path, int statusCode) {
    
    std::ifstream file(requested_path, std::ifstream::binary);
    if (!file.is_open())
        return getErrorResponse(404);
    file.seekg(0, std::ios::end);
    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string body(size, '\0');
    if (!file.read(&body[0], size)) {
        file.close();
        return getErrorResponse(500); // Read error
    }
    file.close();
    return buildResponse(body, statusCode, getMimeType(requested_path));
}

std::string Response::getPostResponse(const std::string& url) {
    std::string resBody;
    std::string uploadedFile;

    if (content_type.empty())
        return getErrorResponse(400); // Bad Request - No Content-Type
    
        // Handle URL-encoded form submission (e.g., /submit)
    if (content_type == "application/x-www-form-urlencoded") {
        if (body.empty())
            return getErrorResponse(400);
        if (url.find("submit") != std::string::npos) {
            resBody = responseApplication(body);
            return buildResponse(resBody, 200, content_type);
        } else
            return getErrorResponse(404); // Not found for this path
    }
    // Handle file uploads (e.g., /uploads)
    if (content_type.find("multipart/form-data") != std::string::npos) {
        if (body.empty())
            return getErrorResponse(400);
        size_t boundary_pos = content_type.find("boundary=");
        if (boundary_pos == std::string::npos)
            return getErrorResponse(400);
        std::string boundary = "--" + content_type.substr(boundary_pos + 9);
        std::string upload_path = "./www/uploads/";
        std::cout << "Upload path: " << upload_path << std::endl;
        struct stat st;
        if (stat(upload_path.c_str(), &st) == -1) {
            if (mkdir(upload_path.c_str(), 0755) == -1)
                return getErrorResponse(500);
        }
        bool success = handleFileUpload(upload_path, body, boundary, uploadedFile);
        if (success)
            return buildResponse(resBody, 200, content_type);
        else
            return getErrorResponse(500); // Upload failed
    }
    return getErrorResponse(415); // Unsupported content-type
}

std::string Response::getDeleteResponse(const std::string& filepath) {
    struct stat st;
    std::cout << "DEBUG: Deleting file: " << filepath << std::endl;
    if (stat(filepath.c_str(), &st) != 0) {
        return getErrorResponse(404); // Not found
    }
    if (remove(filepath.c_str()) != 0) {
        return getErrorResponse(500); // Failed to delete
    }
    return buildResponse("File deleted successfully", 200, "text/plain");
}

std::string Response::getErrorResponse(int statusCode) {
    std::string error_page = "./www/error/" + std::to_string(statusCode) + "_error.html";
    return getGetResponse(error_page, statusCode);
}

std::string Response::getHeadResponse(const std::string& requested_path, int statusCode) {
  // Similar to GET but without body
  std::ifstream file(requested_path, std::ifstream::binary);
  if (!file.is_open())
      return getErrorResponse(404);
  
  file.seekg(0, std::ios::end);
  std::streampos size = file.tellg();
  file.close();
  
  // Create response with headers only
  std::stringstream res;
  res << "HTTP/1.1 " << statusCode << " OK\r\n";
  res << "Content-Type: " << getMimeType(requested_path) << "\r\n";
  res << "Content-Length: " << size << "\r\n";
  res << "\r\n";
  
  return res.str();
}
