#include "../includes/Response.hpp"
#include "../includes/Server.hpp"
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

// Extract script path and PATH_INFO from a URL
void extractScriptAndPathInfo(const std::string& fullPath, std::string& scriptPath, std::string& pathInfo) {
  // Start with the assumption that there's no PATH_INFO
  scriptPath = fullPath;
  pathInfo = "";
  
  // For common extensions like .py, .cgi, .php
  const std::vector<std::string> extensions = {".py", ".cgi", ".php", ".sh"};
  
  for (const auto& ext : extensions) {
      size_t extPos = fullPath.find(ext);
      if (extPos != std::string::npos) {
          // Find the position right after the extension
          size_t pathInfoStart = extPos + ext.length();
          
          // If there's more content after the extension, it's PATH_INFO
          if (pathInfoStart < fullPath.length()) {
              scriptPath = fullPath.substr(0, pathInfoStart);
              pathInfo = fullPath.substr(pathInfoStart);
          }
          break;
      }
  }
}

std::string Response::executeCGI(const std::string& path, const std::string& query, const std::string& method) {
    std::string scriptPath, pathInfo;
    extractScriptAndPathInfo(path, scriptPath, pathInfo);

    // Check if the file exists and is executable
    if (!isCGIScript(path) && !isScriptExtension(path)) {
      std::cout << "DEBUG: Script not found or not executable: " << path << std::endl;
      return getErrorResponse(404);
    }
    // Add debugging output
    std::cout << "DEBUG: Executing CGI script at: " << scriptPath << std::endl;
    std::cout << "DEBUG: PATH_INFO: " << pathInfo << std::endl;
    std::cout << "DEBUG: POST body length: " << body.length() << std::endl;
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
        env_strings.push_back("SCRIPT_NAME=" + scriptPath);
        env_strings.push_back("PATH_INFO=" + pathInfo);
        env_strings.push_back("CONTENT_TYPE=" + content_type);
        
        // // Get content length from headers
        // std::string content_length = "0";
        // for (const auto& header : headers) {
        //     if (header.first == "Content-Length") {
        //         content_length = header.second;
        //         break;
        //     }
        // }
        std::string content_length = std::to_string(body.length());
        env_strings.push_back("CONTENT_LENGTH=" + content_length);

        // Convert string vector to char* array for execve
        char* envp[env_strings.size() + 1];
        for (size_t i = 0; i < env_strings.size(); i++) {
            envp[i] = strdup(env_strings[i].c_str());
        }
        envp[env_strings.size()] = NULL;
        // Execute the CGI script
        char* argv[3]; // Increase size to 3
        if (path.find(".py") != std::string::npos) {
            argv[0] = strdup("/usr/bin/python3"); // Use the interpreter path
            argv[1] = strdup(scriptPath.c_str()); // Script path becomes argument
            argv[2] = NULL;
            execve(argv[0], argv, envp); // Execute the interpreter
        } else {
            // For other scripts, try direct execution
            argv[0] = strdup(path.c_str());
            argv[1] = NULL;
            execve(path.c_str(), argv, envp);
        }

        // If execve returns, there was an error
        perror("execve");
        exit(EXIT_FAILURE);
    }

    // Parent process
    close(pipe_in[0]);  // Close read end of input pipe
    close(pipe_out[1]); // Close write end of output pipe

    // Register with the main server poll loop instead of local polling
    CGIState state = {pid, pipe_in[1], pipe_out[0], body, "", Server::current_client_fd, false};
    Server::cgi_states[pipe_out[0]] = state;
    
    // Add stdout pipe to poll for reading
    Server::poll_fds.push_back({pipe_out[0], POLLIN, 0});
    
    // Add stdin pipe to poll for writing if there's data to send
    if (method == "POST" && !body.empty()) {
      std::cout << "DEBUG: Writing " << body.size() << " bytes to CGI stdin" << std::endl;
      
      // For smaller bodies, try immediate write
      ssize_t written = write(pipe_in[1], body.c_str(), body.size());
      
      if (written == (ssize_t)body.size()) {
        // All data written successfully
        std::cout << "DEBUG: Wrote all " << written << " bytes immediately" << std::endl;
        
        // Close pipe after successful write
        close(pipe_in[1]);
        state.stdin_fd = -1;
        Server::cgi_states[pipe_out[0]] = state;
      }
      else if (written > 0) {
          // Partial write - update the input buffer to only include remaining data
          std::cout << "DEBUG: Partial write of " << written << " bytes" << std::endl;
          state.input_buffer = body.substr(written);
          
          // Add stdin pipe to poll for writing the rest
          Server::poll_fds.push_back({pipe_in[1], POLLOUT, 0});
          Server::cgi_states[pipe_out[0]] = state;
          return ""; // Return empty response - the real response will be sent later
      }
      else if (errno == EAGAIN || errno == EWOULDBLOCK) {
          // No data written due to non-blocking pipe
          std::cout << "DEBUG: Would block, queuing entire body" << std::endl;
          Server::poll_fds.push_back({pipe_in[1], POLLOUT, 0});
          Server::cgi_states[pipe_out[0]] = state;
          return ""; // Return empty response - the real response will be sent later
      }
      else {
          perror("write to CGI stdin");
          
          // Close the pipe if write failed with an error other than would block
          close(pipe_in[1]);
          state.stdin_fd = -1;
          Server::cgi_states[pipe_out[0]] = state;
      }
    }
  return ""; // Return empty string - CGI process is running asynchronously
}
