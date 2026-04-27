#ifndef CONFIGTYPES_HPP
#define CONFIGTYPES_HPP

#include <string>
#include <vector>
#include <map>
#include <set>

struct Listen {
    std::string host; // "127.0.0.1" or "0.0.0.0" l ip dial server
    int port;         // 1 .. 65535 ..... ex: 8080 port dial server
};

struct Redirect {
    bool enabled;
    int code;               // 301/302/...
    std::string target;     // "/new" or full URL
    Redirect() : enabled(false), code(0) {}
};

struct UploadConfig {
    bool enabled;
    std::string dir;
    UploadConfig() : enabled(false) {}
};

typedef std::set<std::string> MethodSet;
typedef std::map<std::string, std::string> CgiMap; // ".py" -> "/usr/bin/python3" ...

struct LocationConfig {
    std::string prefix;                 // "/upload/" "/cgi-bin/" etc.
    MethodSet methods;                  // allowed methods; empty => server/default
    Redirect redirect;                  // optional
    std::string root;                   // optional override
    bool autoindex;                     // default false
    std::vector<std::string> index;     // ["index.html", ...]
    UploadConfig upload;                // optional
    CgiMap cgi;                         // optional

    LocationConfig() : autoindex(false) {}
};

struct ServerConfig {
    std::vector<Listen> listens;                 // interface:port pairs
    std::string server_name;                     // optional
    std::string root;                            // required
    size_t client_max_body_size;                 // bytes; 0 => use default
    std::map<int, std::string> error_pages;      // 404 -> "path"

    std::vector<LocationConfig> locations;

    ServerConfig() : client_max_body_size(0) {}
};

struct Config {
    std::vector<ServerConfig> servers;
};

#endif