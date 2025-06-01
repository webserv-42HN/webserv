#pragma once

#include "../includes/Response.hpp"
#include "../includes/Config_Manager.hpp"

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
