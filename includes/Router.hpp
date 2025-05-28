#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "Response.hpp"
#include "config_manager.hpp"

// class Router {
//     private:
//         std::unordered_map<std::string, RouteHandler> routes;
//         std::vector<ServerConfig> routeConfigs;
//         std::string root;
//         std::map<std::string, RouteConfigFromConfigFile> dynamicRoutes;
//     public:
//         Router(std::vector<ServerConfig> RouteConfigs);
//         ~Router();

//         // t_routeConfig getRouteConfig(std::string url);
//         t_routeConfig getRouteConfig(std::string url);
//         t_routeConfig RootHandler(std::string url);
//         t_routeConfig AboutHandler(std::string url);
//         t_routeConfig DocsHandler(std::string url);
//         t_routeConfig UploadsHandler(std::string url);
//         t_routeConfig SubmitHandler(std::string url);
//         t_routeConfig NotFoundHandler();
//         t_routeConfig cgiHandler(std::string url);

// };

class Router {
    private:
        std::vector<ServerConfig> routeConfigs;
        std::map<std::string, RouteConfigFromConfigFile> dynamicRoutes;
    
        t_routeConfig convertToRouteConfig(const RouteConfigFromConfigFile& cfg);
    
    public:
        Router(std::vector<ServerConfig> RouteConfigs);
        ~Router();
        t_routeConfig getRouteConfig(std::string url);
        t_routeConfig NotFoundHandler();
    };

#endif
