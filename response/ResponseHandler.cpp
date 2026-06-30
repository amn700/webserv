

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

    if (req.status == 301 && !req.confurm_path.empty())
    {
        res.setStatus(301, "Moved Permanently");
        res.setHeader("Location", req.confurm_path);
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
    std::string path = req.confurm_path;
  
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
        return handleAutoIndex(req.redirect_target);
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



// Response ResponseHandler::handlePOST()
// {
//     Response res;

//     // std::cout << "POST target: "
//     //           << req.redirect_target
//     //           << std::endl;

//     std::string filePath = req.confurm_path + "upload.txt";

//     std::ofstream file(filePath.c_str());

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
//     res.setBody("<h1>Upload successful</h1>");
//     res.setHeader("Content-Type", "text/html");

//     return res;
// }

Response ResponseHandler::handlePOST()
{
    Response res;

    std::cout << "=== POST Handler ===\n";
    std::cout << "confurm_path: '" << req.confurm_path << "'\n";
    std::cout << "body size: " << req.body.size() << "\n";

    // Check if it's multipart/form-data
    std::map<std::string, std::string>::const_iterator content_type_it = req.headers.find("Content-Type");
    
    if (content_type_it == req.headers.end())
    {
        res.setStatus(400, "Bad Request");
        res.setBody("<h1>400 Bad Request - No Content-Type</h1>");
        res.setHeader("Content-Type", "text/html");
        return res;
    }

    std::string content_type = content_type_it->second;
    
    if (content_type.find("multipart/form-data") == std::string::npos)
    {
        // Simple form data - just save as is
        std::string filePath = req.confurm_path + "upload.txt";
        std::ofstream file(filePath.c_str());

        if (!file.is_open())
        {
            res.setStatus(500, "Internal Server Error");
            res.setBody("<h1>500 Internal Server Error</h1>");
            res.setHeader("Content-Type", "text/html");
            return res;
        }

        file << req.body;
        file.close();

        res.setStatus(201, "Created");
        res.setBody("<h1>Upload successful</h1>");
        res.setHeader("Content-Type", "text/html");
        return res;
    }

    // Parse boundary
    size_t boundary_pos = content_type.find("boundary=");
    if (boundary_pos == std::string::npos)
    {
        res.setStatus(400, "Bad Request");
        res.setBody("<h1>400 Bad Request - No boundary</h1>");
        res.setHeader("Content-Type", "text/html");
        return res;
    }

    std::string boundary = content_type.substr(boundary_pos + 9);
    std::string delimiter = "--" + boundary;
    std::string end_delimiter = "--" + boundary + "--";

    std::cout << "Boundary: '" << boundary << "'\n";
    std::cout << "Body size: " << req.body.size() << " bytes\n";

    std::string body = req.body;
    size_t pos = 0;
    int file_count = 0;
    int part_count = 0;

    // Find first boundary
    pos = body.find(delimiter);
    if (pos == std::string::npos)
    {
        std::cout << "ERROR: First boundary not found\n";
        res.setStatus(400, "Bad Request");
        res.setBody("<h1>400 Bad Request - Boundary not found</h1>");
        res.setHeader("Content-Type", "text/html");
        return res;
    }

    pos += delimiter.length();

    // Skip initial CRLF
    if (pos < body.length() && body[pos] == '\r') pos++;
    if (pos < body.length() && body[pos] == '\n') pos++;

    // Process each part
    while (pos < body.length())
    {
        part_count++;
        
        // Find next boundary
        size_t next_boundary = body.find("\r\n" + delimiter, pos);
        if (next_boundary == std::string::npos)
            next_boundary = body.find("\n" + delimiter, pos);
        
        if (next_boundary == std::string::npos)
            break;

        // Extract this part (without the boundary marker)
        std::string part = body.substr(pos, next_boundary - pos);

        // Remove trailing CRLF before boundary
        while (!part.empty() && (part.back() == '\r' || part.back() == '\n'))
            part.pop_back();

        std::cout << "\n--- Part " << part_count << " ---\n";
        std::cout << "Part size: " << part.size() << " bytes\n";

        // Find headers/body separator in this part (double CRLF or LF)
        size_t header_end = part.find("\r\n\r\n");
        size_t header_end_lf = part.find("\n\n");
        
        if (header_end == std::string::npos && header_end_lf == std::string::npos)
        {
            std::cout << "ERROR: No header separator found\n";
            pos = next_boundary + 2 + delimiter.length();
            if (pos < body.length() && body[pos] == '\r') pos++;
            if (pos < body.length() && body[pos] == '\n') pos++;
            continue;
        }

        if (header_end == std::string::npos)
            header_end = header_end_lf;
        else if (header_end_lf != std::string::npos && header_end_lf < header_end)
            header_end = header_end_lf;

        std::string part_headers = part.substr(0, header_end);
        
        // Determine how many chars to skip for separator
        int skip = 4;  // \r\n\r\n
        if (part[header_end] == '\n' && part[header_end + 1] == '\n')
            skip = 2;  // \n\n
        
        std::string part_data = part.substr(header_end + skip);

        std::cout << "Headers:\n" << part_headers << "\n";
        std::cout << "Data size: " << part_data.size() << " bytes\n";

        // Extract filename
        size_t filename_pos = part_headers.find("filename=");
        if (filename_pos != std::string::npos)
        {
            // Find the quoted filename
            filename_pos = part_headers.find("\"", filename_pos);
            if (filename_pos != std::string::npos)
            {
                filename_pos++;
                size_t filename_end = part_headers.find("\"", filename_pos);
                std::string filename = part_headers.substr(filename_pos, filename_end - filename_pos);

                std::cout << "Filename: '" << filename << "'\n";

                // Create full path
                std::string filePath = req.confurm_path + filename;
                std::cout << "Saving to: '" << filePath << "'\n";

                // Write file in binary mode
                std::ofstream file(filePath.c_str(), std::ios::binary);

                if (!file.is_open())
                {
                    std::cout << "ERROR: Cannot open file for writing\n";
                    res.setStatus(500, "Internal Server Error");
                    res.setBody("<h1>500 Internal Server Error - Cannot write file</h1>");
                    res.setHeader("Content-Type", "text/html");
                    return res;
                }

                // Write the actual binary data
                file.write(part_data.c_str(), part_data.size());
                file.close();
                file_count++;

                std::cout << "File saved successfully (" << part_data.size() << " bytes)\n";
            }
        }
        else
        {
            // Regular form field
            size_t name_pos = part_headers.find("name=");
            if (name_pos != std::string::npos)
            {
                name_pos = part_headers.find("\"", name_pos);
                if (name_pos != std::string::npos)
                {
                    name_pos++;
                    size_t name_end = part_headers.find("\"", name_pos);
                    std::string name = part_headers.substr(name_pos, name_end - name_pos);

                    std::cout << "Form field: '" << name << "' = '" << part_data << "'\n";
                }
            }
        }

        // Move to next part
        pos = next_boundary + 2;  // Skip \r\n or \n
        if (pos < body.length() && body[pos] == '\r') pos++;
        if (pos < body.length() && body[pos] == '\n') pos++;
        
        pos += delimiter.length();  // Skip boundary
        
        // Skip CRLF after boundary
        if (pos < body.length() && body[pos] == '\r') pos++;
        if (pos < body.length() && body[pos] == '\n') pos++;
    }

    std::cout << "\nTotal parts parsed: " << part_count << "\n";
    std::cout << "Files uploaded: " << file_count << "\n";

    res.setStatus(201, "Created");
    res.setBody("<h1>Upload successful! " + toString(file_count) + " file(s) uploaded</h1>");
    res.setHeader("Content-Type", "text/html");

    return res;
}