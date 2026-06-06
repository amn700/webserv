#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP
#include <map>
#include <iostream>   // for std::cout, std::endl
#include <string>     // for std::string
#include <sstream>    // for std::stringstream
#include <vector> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <string>
#include <sys/stat.h>  // stat, struct stat, S_ISDIR
#include <unistd.h>    // access, R_OK
#include <errno.h>
#include "../configtypes.hpp"

struct validat {
    std::string path; // "127.0.0.1" or "0.0.0.0" l ip dial server
    int code;         // 1 .. 65535 ..... ex: 8080 port dial server
};

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

    HttpRequest(const std::string& raw_request,const ServerConfig& serv);
    validat validate_request(const ServerConfig& serv);
    void reqq();
};


#endif