// #pragma once
// #include <string>
// #include <sstream>

// class Response {
// 	public:
// 		static std::string build(const std::string& body, const std::string& content_type = "text/html", int code = 200, const std::string& status = "OK");
// };

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
#include "Request.hpp"
#include "config_manager.hpp"

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

// enum HttpMethod {
//     GET,
//     POST,
//     DELETE,
//     UNKNOWN
// };

typedef struct RouteConfig {
    std::vector<HttpMethod> allowed_methods;
    std::string root_dir;
    std::string redirect_to;
    std::string default_file;
    bool autoindex;
}   t_routeConfig;

using RouteHandler = std::function<t_routeConfig(std::string)>;
// typedef std::function<t_routeConfig(std::string)> RouteHandler; -> C98++

class Response : public Request
{
    protected:
        std::string error_dir = "./www/error/";
        t_routeConfig route_config;
        std::vector<ServerConfig> Rconfig;
        std::unordered_map<std::string, RouteHandler> routes;

    public:
        Response(std::vector<ServerConfig> config);
        ~Response();

        std::string routing(std::string method, std::string url);
        std::string generatingResponse(HttpMethod method, std::string full_url);
        std::string getGetResponse(const std::string& requested_path, int statusCode);
        std::string getPostResponse(const std::string &path);
        std::string getDeleteResponse(const std::string &path);
        std::string getErrorResponse(int statusCode);

        std::string getHeadResponse(const std::string& requested_path, int statusCode);



        std::string buildResponse(const std::string& body, int statusCode, const std::string& contentType);
        std::string getStatusLine(int statusCode);
        HttpMethod methodToEnum(std::string method);
        std::string generateDirectoryListing(const std::string& path);
        bool isDirectory(const std::string& path);

        bool handleFileUpload(const std::string& path,
                                        const std::string& body,
                                        const std::string& boundary,
                                        std::string& out_filename);
        std::string getMimeType(const std::string& path);
        // size_t getContentLength() const;
        size_t getContentLength(const std::string &headers) const;
        std::string responseApplication(std::string body);
};

#endif