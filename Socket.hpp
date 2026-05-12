#ifndef SOCKET_HPP
#define SOCKET_HPP

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
    Socket(const Socket& other);
    Socket& operator=(const Socket& other);
    ~Socket();

    int get_socket() const;
    struct sockaddr_in get_address() const;
    int get_connection() const;

private:
    struct sockaddr_in address;
    int _socket;
    int connection;
};

typedef std::vector<Socket> Sockets;

#endif
