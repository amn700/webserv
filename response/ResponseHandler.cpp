/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseHandler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mac <mac@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 20:46:44 by naessgui          #+#    #+#             */
/*   Updated: 2026/07/07 11:28:03 by mac              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ResponseHandler.hpp"
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cerrno>

ResponseHandler::ResponseHandler(const HttpRequest& r, const ServerConfig& c)
    : req(r), conf(c)
{
}

std::string toString(int n)
{
    std::stringstream ss;
    ss << n;
    return ss.str();
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
        return "text/plain";

    std::string ext = path.substr(pos_dot);
    if (ext == ".html" || ext == ".htm")
        return "text/html";
    if (ext == ".css")
        return "text/css";
    if (ext == ".js")
        return "application/javascript";
    if (ext == ".png")
        return "image/png";
    if (ext == ".jpg" || ext == ".jpeg")
        return "image/jpeg";
    if (ext == ".gif")
        return "image/gif";
    if (ext == ".txt")
        return "text/plain";
    if (ext == ".pdf")
        return "application/pdf";
    if (ext == ".json")
        return "application/json";

    return "application/octet-stream";
}

std::string getExtFromContentType(const std::string& ct)
{
    if (ct.find("text/plain") != std::string::npos ||
        ct.find("application/x-www-form-urlencoded") != std::string::npos)
        return ".txt";
    if (ct.find("text/html") != std::string::npos)
        return ".html";
    if (ct.find("application/json") != std::string::npos)
        return ".json";
    if (ct.find("image/png") != std::string::npos)
        return ".png";
    if (ct.find("image/jpeg") != std::string::npos || ct.find("image/jpg") != std::string::npos)
        return ".jpg";
    return ".bin";
}

// Joins a directory and a filename, inserting exactly one '/' between them.
static std::string joinPath(const std::string& dir, const std::string& name)
{
    if (dir.empty())
        return name;
    if (dir[dir.size() - 1] == '/')
        return dir + name;
    return dir + "/" + name;
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
    else if (req.status == 413)
    {
        res.setStatus(413, "Payload Too Large");
        res.setBody("<h1>413 Payload Too Large</h1>");
    }
    else if (req.status == 414)
    {
        res.setStatus(414, "URI Too Long");
        res.setBody("<h1>414 URI Too Long</h1>");
    }
    else if (req.status == 501)
    {
        res.setStatus(501, "Not Implemented");
        res.setBody("<h1>501 Not Implemented</h1>");
    }
    else if (req.status == 508)
    {
        res.setStatus(508, "Loop Detected");
        res.setBody("<h1>508 Loop Detected</h1>");
    }
    else
    {
        res.setStatus(500, "Internal Server Error");
        res.setBody("<h1>500 Internal Server Error</h1>");
    }

    res.setHeader("Content-Type", "text/html");

    std::map<int, std::string>::const_iterator it = conf.error_pages.find(req.status);
    if (it != conf.error_pages.end())
    {
        std::string content;
        if (readFile(it->second, content))
        {
            res.setBody(content);
            res.setHeader("Content-Type", getMimeType(it->second));
        }
    }

    return res;
}

Response ResponseHandler::handleGET(const std::string& path)
{
    Response res;
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
    const std::string& path = req.confurm_path;
    struct stat st;

    if (path.empty() || stat(path.c_str(), &st) != 0)
    {
        res.setStatus(404, "Not Found");
        res.setBody("<h1>404 Not Found</h1>");
        res.setHeader("Content-Type", "text/html");
        return res;
    }

    if (S_ISDIR(st.st_mode))
    {
        res.setStatus(403, "Forbidden");
        res.setBody("<h1>403 Forbidden (Directory)</h1>");
        res.setHeader("Content-Type", "text/html");
        return res;
    }

    if (access(path.c_str(), W_OK) != 0)
    {
        res.setStatus(403, "Forbidden");
        res.setBody("<h1>403 Forbidden (No permission)</h1>");
        res.setHeader("Content-Type", "text/html");
        return res;
    }

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

Response ResponseHandler::handlePOST()
{
    Response res;

    if (conf.client_max_body_size > 0 && req.body.size() > conf.client_max_body_size)
    {
        res.setStatus(413, "Payload Too Large");
        res.setBody("<h1>413 Payload Too Large</h1>");
        res.setHeader("Content-Type", "text/html");
        return res;
    }

    const std::string& dir = req.confurm_path;
    if (dir.empty())
    {
        res.setStatus(500, "Internal Server Error");
        res.setBody("<h1>Upload target not configured</h1>");
        res.setHeader("Content-Type", "text/html");
        return res;
    }

    struct stat dir_st;
    if (stat(dir.c_str(), &dir_st) != 0 || !S_ISDIR(dir_st.st_mode))
    {
        res.setStatus(500, "Internal Server Error");
        res.setBody("<h1>Upload directory does not exist</h1>");
        res.setHeader("Content-Type", "text/html");
        return res;
    }

    std::string ext = ".txt";
    std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Type");
    if (it != req.headers.end())
        ext = getExtFromContentType(it->second);

    // Find a free "uploadN.ext" name so concurrent/repeat uploads don't
    // clobber each other.
    std::string fileName;
    int i = 1;
    const int maxAttempts = 100000;
    while (i < maxAttempts)
    {
        fileName = joinPath(dir, "upload" + toString(i) + ext);
        struct stat exist_st;
        if (stat(fileName.c_str(), &exist_st) != 0 && errno == ENOENT)
            break;
        i++;
    }

    if (i >= maxAttempts)
    {
        res.setStatus(500, "Internal Server Error");
        res.setBody("<h1>500 Could not allocate upload filename</h1>");
        res.setHeader("Content-Type", "text/html");
        return res;
    }

    std::ofstream file(fileName.c_str(), std::ios::binary);
    if (!file.is_open())
    {
        res.setStatus(500, "Internal Server Error");
        res.setBody("<h1>500 Internal Server Error</h1>");
        res.setHeader("Content-Type", "text/html");
        return res;
    }

    file.write(req.body.data(), static_cast<std::streamsize>(req.body.size()));
    if (!file)
    {
        res.setStatus(500, "Internal Server Error");
        res.setBody("<h1>500 Internal Server Error</h1>");
        res.setHeader("Content-Type", "text/html");
        return res;
    }
    file.close();

    res.setStatus(201, "Created");
    res.setBody("<h1>Upload successful</h1>");
    res.setHeader("Content-Type", "text/html");

    return res;
}

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
    Response res;

    if (req.status == 1001)
        res = handleAutoIndex(req.confurm_path);
    else if (req.status != 200)
        return handleReqErrors();
    else if (req.method == "GET")
        res = handleGET(req.confurm_path);
    else if (req.method == "DELETE")
        res = handleDELETE();
    else if (req.method == "POST")
        res = handlePOST();
    else
    {
        res.setStatus(405, "Method Not Allowed");
        res.setBody("<h1>405 Method Not Allowed</h1>");
        res.setHeader("Content-Type", "text/html");
        return res;
    }

    if (res.getStatusCode() < 400)
    {
        std::map<std::string, std::string>::const_iterator it = req.query_params.find("user");
        std::string cookieValue;

        if (it != req.query_params.end() && !it->second.empty())
            cookieValue = it->second;
        else
            cookieValue = "visited_homepage";

        res.setCookie("webserv", cookieValue, "/", false);
    }

    return res;
}
