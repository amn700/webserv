#include "Socket.hpp"

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <stdexcept>
#include <string>

static std::string syscallError(const std::string& what)
{
    return what + ": " + ::strerror(errno);
}

static std::string listenKey(const std::string& host, int port)
{
    char tmp[32];
    ::snprintf(tmp, sizeof(tmp), "%d", port);
    return host + ":" + tmp;
}

int Socket::openListenFd(const std::string& host, int port, int backlog)
{
    if (port < 0 || port > 65535)
        throw std::runtime_error("Invalid listen port");
    if (backlog <= 0)
        throw std::runtime_error("Invalid listen backlog");

    struct addrinfo hints;
    struct addrinfo* result = 0;

    ::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    const int rc = ::getaddrinfo(host.c_str(), 0, &hints, &result);
    if (rc != 0 || result == 0 || result->ai_addr == 0) {
        if (result)
            ::freeaddrinfo(result);
        throw std::runtime_error("Invalid listen host: " + host);
    }

    if (result->ai_family != AF_INET || result->ai_addrlen < sizeof(sockaddr_in)) {
        ::freeaddrinfo(result);
        throw std::runtime_error("Invalid listen host: " + host);
    }

    const in_addr addr = ((struct sockaddr_in*)result->ai_addr)->sin_addr;
    ::freeaddrinfo(result);

    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        throw std::runtime_error(syscallError("socket"));

    int opt = 1;
    if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        (void)::close(fd);
        throw std::runtime_error(syscallError("setsockopt(SO_REUSEADDR)"));
    }

    sockaddr_in sa;
    ::memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(static_cast<unsigned short>(port));
    sa.sin_addr = addr;

    if (::bind(fd, (sockaddr*)&sa, sizeof(sa)) < 0) {
        const std::string msg = syscallError("bind(" + listenKey(host, port) + ")");
        (void)::close(fd);
        throw std::runtime_error(msg);
    }

    if (::listen(fd, backlog) < 0) {
        const std::string msg = syscallError("listen(" + listenKey(host, port) + ")");
        (void)::close(fd);
        throw std::runtime_error(msg);
    }

    return fd;
}

Socket* Socket::createListener(const std::string& host, int port, int backlog)
{
    int fd = openListenFd(host, port, backlog);

    sockaddr_in sa;
    ::memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(static_cast<unsigned short>(port));

    // resolve addr again to get the in_addr used by openListenFd
    struct addrinfo hints;
    struct addrinfo* result = 0;
    ::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    const int rc = ::getaddrinfo(host.c_str(), 0, &hints, &result);
    if (rc == 0 && result && result->ai_addr && result->ai_addrlen >= sizeof(sockaddr_in)) {
        sa.sin_addr = ((struct sockaddr_in*)result->ai_addr)->sin_addr;
    }
    if (result)
        ::freeaddrinfo(result);

    return new Socket(fd, sa);
}

Socket::Socket(int domain, int service, int protocol , int port, u_long interface)
{
    memset(&this->address, 0, sizeof(this->address));
    this->address.sin_family = domain;
    this->address.sin_port = htons(port);
    this->address.sin_addr.s_addr = interface;
    this->_socket = socket(domain, service, protocol);
    this->connection = -1;
    if (_socket < 0)
        throw std::runtime_error(syscallError("socket"));
}

Socket::Socket(int fd, const struct sockaddr_in& addr)
    : address(addr), _socket(fd), connection(-1)
{
    if (_socket < 0)
        throw std::runtime_error(syscallError("invalid fd"));
}

// copy ctor / assignment are intentionally private and not implemented

Socket::~Socket()
{
    if (_socket >= 0)
        close(_socket);
}

int Socket::get_socket() const
{
    return this->_socket;
}

struct sockaddr_in Socket::get_address() const
{
    return address;
}

int Socket::get_connection() const
{
    return connection;
}
