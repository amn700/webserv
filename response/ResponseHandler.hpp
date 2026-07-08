#ifndef RESPONSEHANDLER_HPP
# define RESPONSEHANDLER_HPP

#include <string>
#include "response.hpp"
#include "../request/HttpRequest.hpp"
#include "../configtypes.hpp"

class ResponseHandler
{
    public:
        ResponseHandler(const HttpRequest& r, const ServerConfig& c);

        Response handle();

    private:
        const HttpRequest&  req;
        const ServerConfig& conf;

        Response handleGET(const std::string& path);
        Response handleDELETE();
        Response handlePOST();
        Response handleCGI();
        Response handleAutoIndex(const std::string& path);
        Response handleReqErrors();

        ResponseHandler& operator=(const ResponseHandler& other);

bool        readFile(const std::string& path, std::string& content);
std::string getMimeType(const std::string& path);
std::string getExtFromContentType(const std::string& ct);
std::string toString(int n);

#endif
