#include "Socket.hpp"

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <stdexcept>
#include <string>

static std::string syscallError(const std::string& what)
{
    return what + ": " + ::strerror(errno);
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

Socket::Socket(const Socket& other)
    : address(other.address), _socket(-1), connection(other.connection)
{
    if (other._socket >= 0) {
        _socket = dup(other._socket);
        if (_socket < 0)
            throw std::runtime_error(syscallError("dup"));
    }
}

Socket& Socket::operator=(const Socket& other)
{
    if (this == &other)
        return *this;

    if (_socket >= 0)
        close(_socket);

    address = other.address;
    connection = other.connection;
    _socket = -1;

    if (other._socket >= 0) {
        _socket = dup(other._socket);
        if (_socket < 0)
            throw std::runtime_error(syscallError("dup"));
    }
    return *this;
}

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
