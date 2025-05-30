#include "../includes/Response.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <poll.h>
#include <iostream>
#include <cstring>

// Returns true if the file exists and is executable
bool isCGIScript(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        // Check if file exists and is executable
        return (st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH);
    }
    return false;
}

// Check file extension to determine if it's a CGI script
bool isScriptExtension(const std::string& path) {
    return path.find(".cgi") != std::string::npos || 
           path.find(".py") != std::string::npos || 
           path.find(".php") != std::string::npos;
}

std::string Response::executeCGI(const std::string& path, const std::string& query, const std::string& method) {
    // Check if the file exists and is executable
    if (!isCGIScript(path) && !isScriptExtension(path)) {
        std::cout << "DEBUG1: Executing CGI script at path: " << path << std::endl;
        return getErrorResponse(404);
    }
    int pipe_in[2];  // Parent writes to child (CGI input)
    int pipe_out[2]; // Child writes to parent (CGI output)

    if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0) {
        perror("pipe");
        return getErrorResponse(500);
    }

    // Make pipes non-blocking for poll
    fcntl(pipe_in[0], F_SETFL, O_NONBLOCK);
    fcntl(pipe_in[1], F_SETFL, O_NONBLOCK);
    fcntl(pipe_out[0], F_SETFL, O_NONBLOCK);
    fcntl(pipe_out[1], F_SETFL, O_NONBLOCK);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        close(pipe_in[0]);
        close(pipe_in[1]);
        close(pipe_out[0]);
        close(pipe_out[1]);
        return getErrorResponse(500);
    }

    if (pid == 0) { // Child process (CGI script)
        // Close unused pipe ends
        close(pipe_in[1]);  // Close write end of input pipe
        close(pipe_out[0]); // Close read end of output pipe

        // Redirect stdin to read from pipe_in
        dup2(pipe_in[0], STDIN_FILENO);
        close(pipe_in[0]);

        // Redirect stdout to write to pipe_out
        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_out[1]);

        // Prepare environment variables
        // Since we can't use setenv(), we'll build an envp array for execve
        std::vector<std::string> env_strings;
        env_strings.push_back("SERVER_SOFTWARE=WebServ/1.0");
        env_strings.push_back("SERVER_NAME=localhost");
        env_strings.push_back("GATEWAY_INTERFACE=CGI/1.1");
        env_strings.push_back("SERVER_PROTOCOL=HTTP/1.1");
        env_strings.push_back("REQUEST_METHOD=" + method);
        env_strings.push_back("QUERY_STRING=" + query);
        env_strings.push_back("SCRIPT_NAME=" + path);
        env_strings.push_back("CONTENT_TYPE=" + content_type);
        
        // Get content length from headers
        std::string content_length = "0";
        for (const auto& header : headers) {
            if (header.first == "Content-Length") {
                content_length = header.second;
                break;
            }
        }
        env_strings.push_back("CONTENT_LENGTH=" + content_length);

        // Convert string vector to char* array for execve
        char* envp[env_strings.size() + 1];
        for (size_t i = 0; i < env_strings.size(); i++) {
            envp[i] = strdup(env_strings[i].c_str());
        }
        envp[env_strings.size()] = NULL;
        // Execute the CGI script
        char* argv[2];
        argv[0] = strdup(path.c_str());
        argv[1] = NULL;
        execve(path.c_str(), argv, envp);

        // If execve returns, there was an error
        perror("execve");
        exit(EXIT_FAILURE);
    }

    // Parent process
    close(pipe_in[0]);  // Close read end of input pipe
    close(pipe_out[1]); // Close write end of output pipe

    // Send request body to CGI script if it's a POST request
    if (method == "POST" && !body.empty()) {
        // Use poll to handle writing to pipe
        struct pollfd write_fd = {pipe_in[1], POLLOUT, 0};
        size_t bytes_written = 0;
        
        while (bytes_written < body.size()) {
            int poll_result = poll(&write_fd, 1, 5000); // 5 second timeout
            
            if (poll_result < 0) {
                perror("poll");
                break;
            }
            
            if (poll_result > 0 && (write_fd.revents & POLLOUT)) {
                ssize_t result = write(pipe_in[1], body.c_str() + bytes_written, 
                                       body.size() - bytes_written);
                if (result < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        continue; // Try again
                    }
                    perror("write");
                    break;
                }
                bytes_written += result;
            }
        }
    }
    
    // Close write pipe as we're done sending data
    close(pipe_in[1]);

    // Read CGI script output using poll
    std::string cgi_output;
    char buffer[4096];
    bool reading = true;
    struct pollfd read_fd = {pipe_out[0], POLLIN, 0};
    
    while (reading) {
        int poll_result = poll(&read_fd, 1, 5000); // 5 second timeout
        
        if (poll_result < 0) {
            perror("poll");
            break;
        }
        
        if (poll_result == 0) {
            // Timeout
            break;
        }
        
        if (poll_result > 0) {
            if (read_fd.revents & POLLIN) {
                ssize_t bytes_read = read(pipe_out[0], buffer, sizeof(buffer) - 1);
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    cgi_output.append(buffer, bytes_read);
                } else if (bytes_read == 0) {
                    // End of file
                    reading = false;
                } else {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        continue; // Try again
                    }
                    perror("read");
                    break;
                }
            } else if (read_fd.revents & (POLLHUP | POLLERR | POLLNVAL)) {
                // Pipe closed or error
                reading = false;
            }
        }
    }
    
    close(pipe_out[0]);
    
    // Wait for child process to finish
    int status;
    waitpid(pid, &status, 0);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        std::cerr << "CGI script exited with status " << WEXITSTATUS(status) << std::endl;
        return getErrorResponse(500);
    }
    
    // Process CGI output
    // Check if the CGI script returned proper headers
    size_t header_end = cgi_output.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        // No headers found, assume it's just the body with content-type text/html
        return buildResponse(cgi_output, 200, "text/html");
    }
    
    // Extract headers and body
    std::string headers = cgi_output.substr(0, header_end);
    std::string body = cgi_output.substr(header_end + 4);
    
    // Look for Status header
    int status_code = 200;
    std::string content_type = "text/html";
    
    std::istringstream header_stream(headers);
    std::string line;
    while (std::getline(header_stream, line)) {
        if (line.empty() || line == "\r") continue;
        
        // Remove trailing \r if present
        if (!line.empty() && line[line.size()-1] == '\r')
            line.erase(line.size()-1);
        
        if (line.find("Status:") == 0) {
            status_code = std::stoi(line.substr(7));
        } else if (line.find("Content-Type:") == 0) {
            content_type = line.substr(13);
            // Trim leading spaces
            content_type.erase(0, content_type.find_first_not_of(" \t"));
        }
    }
    
    return buildResponse(body, status_code, content_type);
}