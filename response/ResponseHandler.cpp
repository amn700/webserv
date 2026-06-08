

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseHandler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: naessgui <naessgui@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 20:46:44 by naessgui          #+#    #+#             */
/*   Updated: 2026/06/02 20:21:26 by naessgui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ResponseHandler.hpp"
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>


ResponseHandler::ResponseHandler(const HttpRequest& r, const ServerConfig& c) : req(r), conf(c)
{
    
}

std::string toString(int n)
{
    std::stringstream ss; //Reads/writes from a string..

    ss << n;

    return ss.str(); //converts the stream content into a real string.
}



bool readFile(const std::string& path, std::string& content)
{
    std::ifstream file(path.c_str(), std::ios::binary);

    if (!file.is_open())
        return false;

    std::stringstream buffer;
    buffer << file.rdbuf();

    content = buffer.str();
    return true;
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
    if (ext == ".txt")
        return "text/plain";
    if (ext == ".pdf")
        return "application/pdf";
    if (ext == ".json")
        return "application/json";
            
    return ("application/octet-stream");
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
    else if (req.status == 411)
    {
        res.setStatus(411, "Length Required");
        res.setBody("<h1>411 Length Required</h1>");
    }
    else if (req.status == 414)
    {
        res.setStatus(414, "URI Too Long");
        res.setBody("<h1>414 URI Too Long</h1>");
    }
    else if (req.status == 508)
    {
        res.setStatus(508, "Loop Detected");
        res.setBody("<h1>508 Loop Detected</h1>");
    }
    else if (req.status == 501)
    {
        res.setStatus(501, "Not Implemented");
        res.setBody("<h1>501 Not Implemented</h1>");
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
        std::string content;

        if (readFile(it->second, content))
        {
            res.setBody(content);
            res.setHeader("Content-Type", getMimeType(it->second));
        }
        // std::string content = readFile(it->second);

        //     if (!content.empty())
        //     {
        //         res.setBody(content);
        //         res.setHeader("Content-Type", getMimeType(it->second));
        //     }
    }
    else 
    {
        res.setHeader("Content-Type", "text/html");
    }
    return res;
}


Response ResponseHandler::handleGET(const std::string& path)
{
    Response res;
    // struct stat st;
   
    // if (stat(path.c_str(), &st) != 0)
    // {
    //     res.setStatus(404, "Not Found");
    //     res.setBody("<h1>404 Not Found</h1>");
    //     res.setHeader("Content-Type", "text/html");
    //     return res;
    // }
    // if (S_ISDIR(st.st_mode)) //check if directory
    // {
        
    // }
    std::string content;

    if (!readFile(path, content))
    {
        res.setStatus(500, "Internal Server Error");
        res.setBody("<h1>500 Internal Server Error</h1>");
        res.setHeader("Content-Type", "text/html");
        return res;
    }

    res.setStatus(200, "OK");
    res.setBody(content);
    res.setHeader("Content-Type", getMimeType(path));

    return res;
}



Response ResponseHandler::handleDELETE()
{
    Response res;
    std::string path = req.redirect_target;//req.validation.path;
    // struct stat st;

    // if (stat(path.c_str(), &st) != 0)
    // {
    //     res.setStatus(404, "Not Found");
    //     res.setBody("<h1>404 Not Found</h1>");
    //     res.setHeader("Content-Type", "text/html");
    //     return res;
    // }

    // if (S_ISDIR(st.st_mode))
    // {
    //     res.setStatus(403, "Forbidden");
    //     res.setBody("<h1>403 Forbidden (Directory)</h1>");
    //     res.setHeader("Content-Type", "text/html");
    //     return res;
    // }

    // if (access(path.c_str(), W_OK) != 0)
    // {
    //     res.setStatus(403, "Forbidden");
    //     res.setBody("<h1>403 Forbidden (No permission)</h1>");
    //     res.setHeader("Content-Type", "text/html");
    //     return res;
    // }
    if (std::remove(path.c_str()) != 0)
    {
        res.setStatus(500, "Internal Server Error");
        res.setBody("<h1>500 Could not delete file</h1>");
        res.setHeader("Content-Type", "text/html");
        return res;
    }

    res.setStatus(200, "OK");
    res.setBody("<h1>File Deleted Successfully</h1>");
    res.setHeader("Content-Type", "text/html");

    return res;
}


// Response ResponseHandler::handlePOST(const std::string& path)
// {
//     Response res;

//     std::string filename = path + "/upload.txt";

//     std::ofstream file(filename.c_str());

//     if (!file.is_open())
//     {
//         res.setStatus(500, "Internal Server Error");
//         res.setBody("<h1>500 Internal Server Error</h1>");
//         res.setHeader("Content-Type", "text/html");
//         return res;
//     }

//     file << req.body;
//     file.close();

//     res.setStatus(201, "Created");
//     res.setBody("<h1>File Uploaded Successfully</h1>");
//     res.setHeader("Content-Type", "text/html");

//     return res;
// }





Response ResponseHandler::handleAutoIndex(const std::string& path)
{
    Response res;
    DIR* dir = opendir(path.c_str());
    if (!dir)
    {
        res.setStatus(500, "Internal Server Error");
        res.setBody("<h1>500 Internal Server Error</h1>");
        res.setHeader("Content-Type", "text/html");
        return res;
    }

    std::string body;
    body += "<html><body>";
    body += "<h1>Index of " + path + "</h1>";
    body += "<ul>";

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;

        // skip current and parent directory
        if (name == "." || name == "..")
            continue;

        body += "<li><a href=\"" + name + "\">" + name + "</a></li>";
    }

    body += "</ul>";
    body += "</body></html>";

    closedir(dir);

    res.setStatus(200, "OK");
    res.setBody(body);
    res.setHeader("Content-Type", "text/html");

    return res;
}





Response ResponseHandler::handle()
{
    
    if (req.status == 1001)
        return handleAutoIndex(req.confurm_path);
    if (req.status != 200)
        return handleReqErrors();

    if (req.method == "GET")
        return handleGET(req.confurm_path);
    
    else if (req.method == "DELETE")
        return handleDELETE();
    else if (req.method == "POST")
        return handlePOST();
    Response res;

    res.setStatus(405, "Method Not Allowed");
    res.setBody("<h1>405 Method Not Allowed</h1>");
    res.setHeader("Content-Type", "text/html");

    return res;
}

Response ResponseHandler::handlePOST()
{
    Response res;
    res.setStatus(201, "Created");
    res.setBody("<h1>POST Request Placeholder</h1>");
    res.setHeader("Content-Type", "text/html");
    return res;
}

// Response ResponseHandler::handlePOST()
// {
//     const LocationConfig* loc = best_match_location(req.path, conf);

//     Response res;

//     if (!loc || !loc->upload.enabled)
//     {
//         res.setStatus(403, "Forbidden");
//         res.setBody("<h1>403 Forbidden</h1>");
//         res.setHeader("Content-Type", "text/html");
//         return res;
//     }
//     if (loc->upload.dir.empty())
//     {
//         res.setStatus(500, "Internal Server Error");
//         res.setBody("<h1>Upload directory not configured</h1>");
//         res.setHeader("Content-Type", "text/html");
//         return res;
//     }
//     std::string filename = loc->upload.dir + "/upload.txt";
//     struct stat st;

//     if (stat(loc->upload.dir.c_str(), &st) != 0)
//     {
//         res.setStatus(500, "Internal Server Error");
//         res.setBody("<h1>Upload directory does not exist</h1>");
//         res.setHeader("Content-Type", "text/html");
//         return res;
//     }

//     std::ofstream file(filename.c_str());

//     if (!file.is_open())
//     {
//         res.setStatus(500, "Internal Server Error");
//         res.setBody("<h1>500 Internal Server Error</h1>");
//         res.setHeader("Content-Type", "text/html");
//         return res;
//     }

//     file << req.body;
//     file.close();

//     res.setStatus(201, "Created");
//     res.setBody("<h1>File Uploaded Successfully</h1>");
//     res.setHeader("Content-Type", "text/html");

//     return res;
// }