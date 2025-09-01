# webserv

A lightweight HTTP/1.1 web server written in C++. This project implements a configurable, single-binary web server capable of serving static files and handling basic HTTP methods. It is designed for learning purposes and to meet the 42 “webserv” project requirements.

The repository includes:
- A Makefile to build the server
- Example configuration files: `webserv.conf`, `webserv1.conf`
- A `www/` directory with demo assets
- A `YoupiBanane/` directory with additional test content
- `src/` and `includes/` with the server implementation

## Quick start

Build:
```bash
make
```

Run with the default config:
```bash
./webserv
# or explicitly:
./webserv webserv.conf
```

Try a request (example host/port matches the provided sample config):
```bash
curl -X GET -H "Host: example.com" http://127.0.0.1:8080
```

Stop the server with Ctrl+C.

## Requirements

- Unix-like OS (Linux or macOS)
- A C++ compiler (g++ or clang++)
- make
- Optionally: curl (for quick testing)

## Building

Common Make targets:
```bash
make         # build the server
make re      # rebuild from scratch
make clean   # remove object files
make fclean  # remove objects and the binary
```

The output binary is typically `./webserv` in the repository root.

## Running

Basic usage:
```bash
./webserv [path/to/config.conf]
```

- If no configuration path is provided, the server attempts to use a default (e.g., `webserv.conf`).
- Example configs are provided: `webserv.conf` and `webserv1.conf`.

## Configuration

Configuration files define one or more servers (virtual hosts) and their routes. The provided examples include common directives such as:

- listen: Port (and optionally address) to bind, e.g., 8080
- server_name: Hostname(s) the server responds to (via the Host header)
- root: Document root (e.g., `./www`)
- index: Default index file(s) for directories
- error_page: Custom error pages by status code
- client_max_body_size: Maximum request body size
- locations/paths: Per-path configuration (allowed methods, autoindex, redirections, uploads, CGI, etc.)

Open `webserv.conf` to see the full syntax and adapt it to your needs.

## Features

- HTTP/1.1 compliant request parsing and response formatting
- Static file serving from a configurable document root
- Basic method handling (e.g., GET; additional methods depend on configuration)
- Virtual hosting via server blocks and Host header
- Per-location overrides (e.g., indexes, autoindex, uploads, CGI)
- Custom error pages
- Configurable client body size limits

Note: Exact feature set depends on your configuration and the compiled binary.

## Project structure

```
.
├─ Makefile
├─ README.md
├─ LICENSE
├─ webserv.conf
├─ webserv1.conf
├─ www/             # sample site content (document root)
├─ YoupiBanane/     # additional test assets
├─ includes/        # public headers
└─ src/             # implementation
```

## Testing

Try basic requests against the sample configuration:

- Root document:
```bash
curl -i -H "Host: example.com" http://127.0.0.1:8080/
```

- Static file:
```bash
curl -i -H "Host: example.com" http://127.0.0.1:8080/path/to/file.ext
```

- Non-existing path (to see error handling):
```bash
curl -i -H "Host: example.com" http://127.0.0.1:8080/does-not-exist
```

If you change the listen port or server_name in your config, update the curl URL and Host header accordingly.

## Logging and errors

- The server prints informational and error messages to the console.
- Use your shell’s redirection to capture logs:
```bash
./webserv webserv.conf >webserv.out.log 2>webserv.err.log
```

## Troubleshooting

- Port already in use: stop the process using the port or change the listen directive in the config.
- 400/404/405 errors: verify the requested path, allowed methods, and location blocks in your config.
- Host mismatch: ensure your curl Host header matches a configured server_name (or adjust server_name).
- Permission errors: check file permissions under `www/` and the working directory.

## License

This project is released under the MIT License. See `LICENSE` for details.

## Acknowledgements

- 42 School “webserv” project
- RFC 7230/7231 (HTTP/1.1) specifications
