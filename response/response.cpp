/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: naessgui <naessgui@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 13:02:04 by naessgui          #+#    #+#             */
/*   Updated: 2026/06/06 12:13:40 by naessgui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "response.hpp"

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

static std::string toString(int n)
{
    std::stringstream ss; //Reads/writes from a string..

    ss << n;

    return ss.str(); //converts the stream content into a real string.
}
std::string Response::buildResponse()
{
    std::string response;

    headers["Content-Length"] = toString(body.size());

    response += "HTTP/1.1 ";
    response += toString(statusCode);
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