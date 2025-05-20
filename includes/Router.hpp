#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "../includes/Response.hpp"

class Router : public Response {
    private:
        std::unordered_map<std::string, RouteHandler> routes;
    public:
        Router();
        ~Router();
        
        t_responseRouteConfig getResponseRouteConfig(std::string url);
        t_responseRouteConfig RootHandler(std::string url);
        t_responseRouteConfig AboutHandler(std::string url);
        t_responseRouteConfig DocsHandler(std::string url);
        t_responseRouteConfig UploadsHandler(std::string url);
        t_responseRouteConfig SubmitHandler(std::string url);
        t_responseRouteConfig NotFoundHandler();
};

#endif
