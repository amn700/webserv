/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseHandler.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: naessgui <naessgui@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 20:47:01 by naessgui          #+#    #+#             */
/*   Updated: 2026/06/03 20:23:19 by naessgui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef RESPONSEHANDLER_HPP
#define RESPONSEHANDLER_HPP
#include "response.hpp"
#include "../request/HttpRequest.hpp"
#include "../configtypes.hpp"



class ResponseHandler
{
    private:
        const HttpRequest& req;
        const ServerConfig& conf;

    public:
        ResponseHandler(const HttpRequest& r, const ServerConfig& c);

        Response handle();

    private:
        Response handleReqErrors();

        Response handleGET(const std::string& path);
        Response handlePOST();
        Response handleDELETE();
        Response handleAutoIndex(const std::string& path);

};
#endif