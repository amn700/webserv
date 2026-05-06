#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP
#include <map>
#include <iostream>   // for std::cout, std::endl
#include <string>     // for std::string
#include <sstream>    // for std::stringstream
#include <vector> 
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <string>
#include <sys/stat.h>  // stat, struct stat, S_ISDIR
#include <unistd.h>    // access, R_OK
#include <errno.h>

#include "../configtypes.hpp"


// //not mine 
// typedef std::set<std::string> MethodSet;
// struct UploadConfig {
//     bool enabled;
//     std::string dir;
//     UploadConfig() : enabled(false) {}
// };

// struct LocationConfig {
//     std::string prefix;                 // "/upload/" "/cgi-bin/" etc.
//     MethodSet methods;                  // allowed methods; empty => server/default
//     Redirect redirect;                  // optional
//     std::string root;                   // optional override
//     bool autoindex;                     // default false
//     std::vector<std::string> index;     // ["index.html", ...]
//     UploadConfig upload;                // optional
//     CgiMap cgi;                         // optional

//     LocationConfig() : autoindex(false) {}
// };

// struct ServerConfig {
//     std::vector<Listen> listens;                 // interface:port pairs
//     std::string server_name;                     // optional
//     std::string root;                            // required
//     size_t client_max_body_size;                 // bytes; 0 => use default
//     std::map<int, std::string> error_pages;      // 404 -> "path"

//     std::vector<LocationConfig> locations;

//     ServerConfig() : client_max_body_size(0) {}
// };
// //



class HttpRequest {
public:
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    std::map<std::string, std::string> query_params;

    int status; // set to 200, 404, 403, 405, 301...
    std::string redirect_target; // empty unless redirect

    HttpRequest(const std::string& raw_request, ServerConfig serv);
    int validate_request(ServerConfig serv);
};

// class ResponseBuilder
// {
// public:
//     std::string build(const HttpRequest& request)
//     {
//         // Handle different methods
//         if (request.method == "GET")
//         {
//             return handle_get(request);
//         } 
//         else if (request.method == "POST")
//          {
//             return handle_post(request);
//         }
//         else if (request.method == "HEAD")
//         {
//             return handle_head(request);
//         }
//         else 
//         {
//             return error_type();  // Method Not Allowed
//         }
//     }
    
// private:
//     std::string handle_get(const HttpRequest& req) 
//     {
//         return response;
//     }
    
//     std::string handle_post(const HttpRequest& req)
//     {
//         return response;
//     }
    
//     std::string handle_head(const HttpRequest& req)
//     {
//         return response;
//     }
    
//     std::string error_404()
//     {
//         return response;
//     }
    
//     std::string error_405()
//     {
//         return response;
//     }
// };

// ResponseBuilder response_builder;
// std::string http_response = response_builder.build(request);
#endif