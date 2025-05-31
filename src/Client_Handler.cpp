#include "../includes/Server.hpp"

bool Server::receiveData(int client_fd) {
    char buf[BUF_SIZE];
    ssize_t nread = recv(client_fd, buf, BUF_SIZE - 1, 0);
    if (nread <= 0)
        return false;

    client_sessions[client_fd].buffer.append(buf, nread);
    return true;
}

bool Server::processHeaders(ClientSession& session) {
    size_t header_end = session.buffer.find("\r\n\r\n");
    if (!session.headers_received && header_end != std::string::npos) {
        Response res(config);
        session.headers_received = true;
        header_end += 4;
        std::string headers = session.buffer.substr(0, header_end);
        session.content_length = res.getContentLength(headers);
    } else if (!session.headers_received) {
        return false; // Headers not fully received yet
    }
    return true;
}

bool Server::isFullRequestReceived(const ClientSession& session) {
    size_t header_end = session.buffer.find("\r\n\r\n");
    if (header_end == std::string::npos)
        return false;

    header_end += 4;
    return session.buffer.size() >= header_end + session.content_length;
}

void Server::processRequest(int client_fd, ClientSession& session) {
    
    size_t header_end = session.buffer.find("\r\n\r\n");
    header_end += 4;
    std::string header_str = session.buffer.substr(0, header_end);
    std::string full_request = session.buffer;
    std::string response;
    Response res(config);
    std::string host = getHostFromHeaders(header_str);
    int port = getPortFromHeaders(header_str);
    const ServerConfig* server_cfg = getServerConfigByHost(config, host, port);

    if (!server_cfg && !host.empty()) {
        response = res.getErrorResponse(404); // Not Found
        responses[client_fd] = response;
        client_sessions.erase(client_fd);
        enableWriteEvents(client_fd);
        return;
    } else if (!server_cfg) {
        server_cfg = &config[0];
    }
    Response real_res({*server_cfg});

    if (real_res.isMalformedRequest(full_request)) {
        response = real_res.getErrorResponse(400);
    } else {
        real_res.parseRequest(full_request);
        if (real_res.getContentLength(header_str) > config[0].client_max_body_size)
            response = real_res.getErrorResponse(413); // Payload Too Large
        else
            response = real_res.routing(real_res.getRequestLine().method, real_res.getRequestLine().url);
    }

    responses[client_fd] = response;
    client_sessions.erase(client_fd);
    enableWriteEvents(client_fd);
}

void Server::enableWriteEvents(int client_fd) {
    for (auto& pfd : poll_fds) {
        if (pfd.fd == client_fd) {
            pfd.events = POLLOUT;
            pfd.revents = 0;
            break;
        }
    }
}
