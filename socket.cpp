#include "webserv.hpp"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <cstdlib>


Socket::Socket(int domain, int service, int protocol , int port, u_long interface)
{
    this->address.sin_family = domain;
    this->address.sin_port = htons(port);
    this->address.sin_addr.s_addr = interface;
    this->_socket = socket(domain, service, protocol);
    this->connection = -1;
    //did socket worked and what if it didnt 
    // mazal ma3rftch XD
    if (_socket < 0)
        std::exit(1);
}

Socket::~Socket(){};

int Socket::get_socket()
{
    return this->_socket;
}

struct sockaddr_in Socket::get_address()
{
    return address;
}

int Socket::get_connection()
{
    return connection;
}
