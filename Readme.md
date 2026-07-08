# WebServ

A custom HTTP web server implemented in C++ using POSIX sockets and `poll()` for event-driven I/O.

## Overview

WebServ is a standalone HTTP server designed for learning and experimentation. It supports:

- multiple virtual hosts
- location-based routing
- static file serving
- CGI script execution
- file uploads
- directory auto-indexing
- custom error pages
- request size limiting
- GET / POST / DELETE methods

This implementation is written in C++98 and depends only on the standard library and POSIX APIs.

## Project Layout

- `main.cpp` — program entry point
- `WebServer.cpp` / `WebServer.hpp` — listener and client management, `poll()` loop
- `configloader.cpp` / `configloader.hpp` — configuration parsing and validation
- `configtypes.hpp` — configuration data structures
- `listen_parser.cpp` — parses `listen` directives
- `location_parsing.cpp` — parses `location` blocks
- `socket.cpp` / `socket.hpp` — socket creation and address resolution
- `request/HttpRequest.cpp` / `request/HttpRequest.hpp` — HTTP request parsing and validation
- `response/response.cpp` / `response/response.hpp` — HTTP response construction
- `response/ResponseHandler.cpp` / `response/ResponseHandler.hpp` — request dispatch and response generation
- `cookies/Cookie.cpp` / `cookies/Cookie.hpp` — cookie parsing and Set-Cookie header building
- `configurations/webserv.conf` — sample server configuration
- `www/` — sample website content and CGI scripts
- `Makefile` — build rules

## Build

From the repository root, run:

```bash
make
```

This builds the `webserv` executable.

To remove compiled objects:

```bash
make clean
```

To rebuild from scratch:

```bash
make re
```

## Run

Start the server with a configuration file:

```bash
./webserv configurations/webserv.conf
```

The sample configuration contains two server blocks and several example locations.

## Configuration

The server uses an nginx-style configuration format. A `server` block may include:

- `listen <host>:<port>;`
- `server_name <name>;`
- `root <path>;`
- `client_max_body_size <bytes>;`
- `error_page <code> <path>;`
- `location <prefix> { ... }`

### Location block directives

Supported location directives:

- `allowed_methods <list>;`
- `return <code> <target>;`
- `root <path>;`
- `autoindex on|off;`
- `index <file> ...;`
- `upload on|off;`
- `upload_dir <path>;`
- `cgi_ext <extension> <interpreter>;`

## Features

### Virtual hosts

Multiple `server` blocks may listen on different addresses and ports. If multiple servers share the same listener, the `Host` header is used to select the matching server.

### Static file serving

Requests map to filesystem resources under the configured `root` or a location-specific `root`. If a directory is requested, the server attempts to serve an index file or generates a directory listing when `autoindex on` is enabled.

### CGI execution

Locations configured with `cgi_ext` execute matching files using the configured interpreter. Example:

```nginx
location /cgi-bin {
  root ./www/site1/cgi-bin/cgi-bin;
  cgi_ext .py /usr/bin/python3;
  cgi_ext .sh /bin/bash;
}
```

CGI execution passes request data through stdin and constructs a response from the script output.

### File uploads

Upload support is enabled with `upload on;` and `upload_dir <path>;` inside a location block.

The server writes POST bodies to the configured upload directory and generates safe filenames.

### Custom error pages

Use `error_page <code> <path>;` to define custom error pages for common error statuses.

## Example config

```nginx
server {
  listen 127.0.0.1:8080;
  listen 127.255.255.2:8084;
  listen 127.255.255.2:8083;

  server_name site1.local;
  root ./www/site1;
  client_max_body_size 100000000;

  error_page 404 ./www/site1/errors/404.html;
  error_page 403 ./www/site1/errors/403.html;
  error_page 500 ./www/site1/errors/500.html;

  location / {
    index index.html;
    autoindex off;
  }

  location /assets {
    allowed_methods GET;
    root ./www/site1/static;
    autoindex off;
  }

  location /list {
    allowed_methods GET;
    root ./www/site1/public-list;
    autoindex on;
  }

  location /old/ {
    allowed_methods GET;
    return 301 /new/;
  }

  location /new {
    allowed_methods GET;
    root ./www/site1/pages;
    index new_page.html new.html;
    autoindex on;
  }

  location /upload {
    allowed_methods GET POST;
    upload on;
    upload_dir ./www/site1/uploads;
  }

  location /uploads {
    allowed_methods GET DELETE;
    root ./www/site1;
    autoindex on;
  }

  location /cgi-bin {
    allowed_methods GET POST;
    root ./www/site1/cgi-bin/cgi-bin;
    cgi_ext .py /usr/bin/python3;
    cgi_ext .sh /bin/bash;
  }
}

server {
  listen 127.255.255.2:9090;
  server_name site2.local;
  root ./www/site2;
  client_max_body_size 200000;
}
```

## Usage examples

### GET request

```bash
curl http://127.0.0.1:8080/
```

### Follow a redirect

```bash
curl -L http://127.0.0.1:8080/old/
```

### Upload a file

```bash
curl -X POST -H "X-File-Name: example.txt" --data-binary @example.txt http://127.0.0.1:8080/upload
```

### Delete a file

```bash
curl -X DELETE http://127.0.0.1:8080/uploads/example.txt
```

### CGI request

```bash
curl http://127.0.0.1:8080/cgi-bin/hello.py
```

## Notes

- The server is single-threaded.
- It uses non-blocking sockets and `poll()` for event multiplexing.
- It is intended for experimentation and learning rather than production deployment.

## Build targets

- `make` — build the server
- `make clean` — remove object files
- `make re` — clean and rebuild

## License

This repository contains an educational C++ web server implementation.
