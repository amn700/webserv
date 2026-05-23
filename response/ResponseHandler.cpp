/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseHandler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: naessgui <naessgui@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 20:46:44 by naessgui          #+#    #+#             */
/*   Updated: 2026/05/23 22:47:20 by naessgui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ResponseHandler.hpp"
#include <sstream>
#include <fstream>


ResponseHandler::ResponseHandler(const HttpRequest& r, const ServerConfig& c) : req(r), conf(c)
{
    
}

std::string toString(int n)
{
    std::stringstream ss; //Reads/writes from a string..

    ss << n;

    return ss.str(); //converts the stream content into a real string.
}


std::string readFile(const std::string& path)
{
    std::ifstream file(path.c_str());

    if (!file.is_open())
        return "";

    std::stringstream buffer;

    buffer << file.rdbuf(); //copy entire file into buffer

    return buffer.str(); //Returns the whole content as a string.
}


Response ResponseHandler::handleReqErrors()
{
    Response res;

    if (req.status == 301 && !req.redirect_target.empty())
    {
        res.setStatus(301, "Moved Permanently");
        res.setHeader("Location", req.redirect_target);
        res.setHeader("Content-Type", "text/html");
        res.setBody("<h1>301 Moved Permanently</h1>");
    }
    else if (req.status == 400)
    {
        res.setStatus(400, "Bad Request");
        res.setBody("<h1>400 Bad Request</h1>");
    }
    else if (req.status == 403)
    {
        res.setStatus(403, "Forbidden");
        res.setBody("<h1>403 Forbidden</h1>");
    }
    else if (req.status == 404)
    {
        res.setStatus(404, "Not Found");
        res.setBody("<h1>404 Not Found</h1>");
    }
    else if (req.status == 405)
    {
        res.setStatus(405, "Method Not Allowed");
        res.setBody("<h1>405 Method Not Allowed</h1>");
    }
    else if (req.status == 413)
    {
        res.setStatus(413, "Payload Too Large");
        res.setBody("<h1>413 Payload Too Large</h1>");
    }
    else if (req.status == 500)
    {
        res.setStatus(500, "Internal Server Error");
        res.setBody("<h1>500 Internal Server Error</h1>");
    }

    std::map<int, std::string>::const_iterator it; 

    it = conf.error_pages.find(req.status);

    if (it != conf.error_pages.end())
    {
        std::string content = readFile(it->second);

            if (!content.empty())
            {
                res.setBody(content);
                res.setHeader("Content-Type", getMimeType(it->second));
            }
    }
    else 
    {
        res.setHeader("Content-Type", "text/html");
        res.setBody("<h1>Error</h1>");
    }
    return res;
}
std::string getMimeType(const std::string& path)
{
    size_t pos_dot = path.find_last_of('.');
    
    if (pos_dot == std::string::npos)
        return ("text/plain");
        
    std::string ext = path.substr(pos_dot);
    if (ext == ".html" || ext == ".htm")
        return ("text/html");
    if (ext == ".css")
        return ("text/css");
    if (ext == ".js")
        return ("application/javascript");
    if (ext == ".png")
        return ("image/png");
    if (ext == ".jpg" || ext == ".jpeg")
        return ("image/jpeg");
    if (ext == ".gif")
        return ("image/gif");
        
    return ("application/octet-stream");
}

Response ResponseHandler::handleGET(const std::string& path)
{
    Response res;

    std::string content = readFile(path);

    if (content.empty())
    {
        res.setStatus(404, "Not Found");
        res.setBody("<h1>404 Not Found</h1>");

    }
    else
    {
        res.setStatus(200, "OK");
        res.setBody(content);

    }

    res.setHeader("Content-Type", getMimeType(path));

    return res;
}
Response ResponseHandler::handle()
{
    if (req.status != 200)
        return handleReqErrors();

    if (req.method == "GET")
        return handleGET(req.redirect_target);

    Response res;

    res.setStatus(405, "Method Not Allowed");
    res.setBody("<h1>405 Method Not Allowed</h1>");
    res.setHeader("Content-Type", "text/html");

    return res;
}
