#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>


class Socket
{
public:
    Socket(
            int domain,
            int service,
            int protocol,
            int port,
            u_long interface
        );
    ~Socket();
    int get_socket();
    struct sockaddr_in get_address();
    int get_connection();
    // virtual void connect_to_network() = 0;
private:
    struct sockaddr_in address;
    int _socket;
    int connection;
};

typedef std::vector<Socket> Sockets;

#endif

