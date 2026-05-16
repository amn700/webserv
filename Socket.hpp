#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <vector>
#include <netinet/in.h>
#include <string>

class Socket {
public:
    Socket(const std::string& host, int port);
    ~Socket();

    int         get_fd()      const;
    sockaddr_in get_address() const;

private:
    int         _fd;
    sockaddr_in _addr;
};

typedef std::vector<Socket*> Sockets;

#endif
