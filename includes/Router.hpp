#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "Response.hpp"

class Router : public Response {
    private:
        std::unordered_map<std::string, RouteHandler> routes;
    public:
        Router();
        ~Router();

        t_routeConfig getRouteConfig(std::string url);
        t_routeConfig RootHandler(std::string url);
        t_routeConfig AboutHandler(std::string url);
        t_routeConfig DocsHandler(std::string url);
        t_routeConfig UploadsHandler(std::string url);
        t_routeConfig SubmitHandler(std::string url);
        t_routeConfig NotFoundHandler();
        t_routeConfig cgiHandler(std::string url);
};

#endif
