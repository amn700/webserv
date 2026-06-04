/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: naessgui <naessgui@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 13:01:03 by naessgui          #+#    #+#             */
/*   Updated: 2026/05/23 22:50:57 by naessgui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <iostream>


class Response
{
     private:
        int statusCode;
        std::string statusMessage;
        std::string body;
        std::map<std::string, std::string> headers;
        
    public:
        Response();
        void setStatus(int statusCode , std::string statusMessage);
        void setBody(std::string content);
        void setHeader(std::string key, std::string value);
        std::string getBody() const;
        
        std::string buildResponse();
};

#endif