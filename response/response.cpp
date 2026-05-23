/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: naessgui <naessgui@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 13:02:04 by naessgui          #+#    #+#             */
/*   Updated: 2026/05/23 21:47:22 by naessgui         ###   ########.fr       */
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

std::string Response::buildResponse()
{
    std::string response;

    headers["Content-Length"] = std::to_string(body.size());

    response += "HTTP/1.0 ";
    response += std::to_string(statusCode);
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