#include "Server.hpp"

#include <errno.h>
#include <netdb.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string.h>

static std::string syscallError(const std::string& what)
{
    return what + ": " + ::strerror(errno);
}

static std::string toStringInt(int v)
{
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

Server::Server(const ServerConfig& config): _config(config)
{
}

void Server::setup()
{
    _sockets.clear();
    _sockets.reserve(_config.listens.size());

    for (size_t i = 0; i < _config.listens.size(); ++i) {
        const Listen& l = _config.listens[i];

        in_addr addr;
        struct addrinfo hints, *result;

        ::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(l.host.c_str(), NULL, &hints, &result) != 0)
            throw std::runtime_error("Invalid listen host: " + l.host);
        addr = ((struct sockaddr_in*)result->ai_addr)->sin_addr;
        freeaddrinfo(result);

        Socket sock(AF_INET, SOCK_STREAM, 0, l.port, addr.s_addr);

        int opt = 1;
        if (::setsockopt(sock.get_socket(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
            throw std::runtime_error(syscallError("setsockopt(SO_REUSEADDR)"));

        sockaddr_in sa = sock.get_address();
        if (::bind(sock.get_socket(), (sockaddr*)&sa, sizeof(sa)) < 0)
            throw std::runtime_error(syscallError("bind(" + l.host + ":" + toStringInt(l.port) + ")"));

        if (::listen(sock.get_socket(), 128) < 0)
            throw std::runtime_error(syscallError("listen"));

        _sockets.push_back(sock);
    }
}

const ServerConfig& Server::config() const
{
    return _config;
}

const Sockets& Server::sockets() const
{
    return _sockets;
}
