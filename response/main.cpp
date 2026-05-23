/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: naessgui <naessgui@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 16:23:42 by naessgui          #+#    #+#             */
/*   Updated: 2026/05/23 22:50:11 by naessgui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "response.hpp"


#include "ResponseHandler.hpp"
#include <iostream>


int main()
{
    ServerConfig conf;
    conf.root = "../www";

    std::string fakeRequest = "GET /404.html HTTP/1.1";

    HttpRequest req(fakeRequest, conf);

    ResponseHandler handler(req, conf);

    Response res = handler.handle();

    std::cout << res.buildResponse() << std::endl;
}
