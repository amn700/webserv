#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP
#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include "../cookies/Cookie.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "../configtypes.hpp"

struct validat {
    std::string path;
    int code;
};

class HttpRequest {
public:
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> cookies;
    std::string body;
    std::map<std::string, std::string> query_params;

    int status;
    std::string redirect_target;
    std::string confurm_path;
    bool is_cgi;
    std::string query_string;
    std::string cgi_script_path;
    std::string cgi_extension;
    std::string cgi_interpreter;
    std::map<std::string, std::string> cgi_env;

    HttpRequest(const std::string& raw_request, const ServerConfig& serv);
    validat validate_request(const ServerConfig& serv);
    bool detect_cgi_request(const ServerConfig& serv);
    void setup_cgi_environment(const ServerConfig& serv, int serverPort);
    void reqq();
};


#endif