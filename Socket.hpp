#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <sys/socket.h>
#include <netinet/in.h>

#include <string>
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

    // Adopt an existing file descriptor (takes ownership).
    Socket(int fd, const struct sockaddr_in& addr);

    ~Socket();

    // Helper used by the server: create/bind/listen a TCP socket on host:port.
    // Returns the created fd (caller owns it and must close it).
    static int openListenFd(const std::string& host, int port, int backlog);

    // Convenience for WebServer: creates and returns an owning Socket*.
    // Caller owns the returned pointer and must delete it.
    static Socket* createListener(const std::string& host, int port, int backlog);

    int get_socket() const;
    struct sockaddr_in get_address() const;
    int get_connection() const;

private:
    Socket(const Socket& other);
    Socket& operator=(const Socket& other);

    struct sockaddr_in address;
    int _socket;
    int connection;
};

typedef std::vector<Socket*> Sockets;

#endif
