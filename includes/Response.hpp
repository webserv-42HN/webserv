#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <sys/stat.h>
#include <dirent.h>
#include "../includes/Request.hpp"

/* 
HTTP Status Codes
- 1xx: Informational - Request received, continuing process
- 2xx: Success - The action was successfully received, understood, and accepted
- 3xx: Redirection - Further action must be taken in order to complete the request
- 4xx: Client Error - The request contains bad syntax or cannot be fulfilled
- 5xx: Server Error - The server failed to fulfill an apparently valid request
*/

enum StatusCode {
    OK = 200,
    BadRequest = 400,
    NotFound = 404,
    InternalServerError = 500
};

enum HttpMethod {
    GET,
    POST,
    DELETE,
    UNKNOWN
};

typedef struct RouteConfig {
    std::vector<HttpMethod> allowed_methods;
    std::string root_dir;
    std::string redirect_to;
    bool autoindex;
}   t_routeConfig;

using RouteHandler = std::function<t_routeConfig(std::string)>;
// typedef std::function<t_routeConfig(std::string)> RouteHandler; -> C98++

class Response : public Request
{
    protected:
        std::string error_dir = "./www/error/";
        t_routeConfig route_config;
        std::unordered_map<std::string, RouteHandler> routes;

    public:
        Response();
        ~Response();
        
        std::string routing(std::string method, std::string url);
        std::string generatingResponse(HttpMethod method, std::string full_url);
        std::string getGetResponse(std::string path, int statusCode);
        std::string getPostResponse(std::string path);
        std::string getDeleteResponse(std::string path);
        std::string getErrorResponse(int statusCode);    
        
        std::string buildResponse(std::string body, int statusCode);
        std::string getStatusLine(int statusCode);
        HttpMethod methodToEnum(std::string method);
        std::string generateDirectoryListing(const std::string& path);
        bool isDirectory(const std::string& path);
        bool handleFileUpload(const std::string& path, const std::string& body, const std::string& boundary);
};

#endif