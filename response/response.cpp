/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mac <mac@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 13:02:04 by naessgui          #+#    #+#             */
/*   Updated: 2026/06/04 08:20:54 by mac              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "response.hpp"

// std::string intToString(int v);

Response::Response()
{
}

void Response::setStatus(int code, std::string message)
{
    statusCode = code;
    statusMessage = message;
}

void Response::setBody(std::string content)
{
    body = content;
}
void Response::setHeader(std::string key, std::string value)
{
    headers[key] = value;
}

std::string Response::getBody() const
{
    return body;
}

void Response::print() const
{
    std::cout << "===== HTTP RESPONSE =====" << std::endl;
    std::cout << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
        it != headers.end(); ++it)
    {
        std::cout << it->first << ": " << it->second << "\r\n";
    }
    std::cout << "Content-Length: " << body.size() << "\r\n";
    std::cout << "\r\n";
    std::cout << body << std::endl;
    std::cout << "=========================" << std::endl;
}

#include <sstream>

std::string intToString(int v)
{
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

std::string Response::buildResponse()
{
    std::string response;

    headers["Content-Length"] = intToString(body.size());

    response += "HTTP/1.1 ";
    response += intToString(statusCode);
    response += " ";
    response += statusMessage;
    response += "\r\n";

    for (std::map<std::string, std::string>::iterator it = headers.begin();
        it != headers.end(); ++it)
    {
        response += it->first;
        response += ": ";
        response += it->second;
        response += "\r\n";
    }

    response += "\r\n";

    response += body;

    return response;
}