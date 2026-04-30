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
class HttpRequest
{
    public:
    std::string method;       // 1. What to do (GET, POST, etc.)
    std::string path;          // 2. What resource (/index.html)
    std::string version;       // 3. HTTP version (HTTP/1.1)
    std::map<std::string, std::string> headers;  // 4. Extra info (Host, User-Agent, etc.)
    std::string body;          // 5. Data being sent (for POST)
    std::map<std::string, std::string> query_params;
    HttpRequest (const std::string& raw_request);
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