git #include "WebServer.hpp"

#include <sys/socket.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <stdexcept>

static std::string syscallError(const std::string& what)
{
    return what + ": " + ::strerror(errno);
}

void WebServer::setNonBlocking(int fd)
{
    int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0)
        throw std::runtime_error(syscallError("fcntl(F_GETFL)"));

    if ((flags & O_NONBLOCK) != 0)
        return;

    if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
        throw std::runtime_error(syscallError("fcntl(F_SETFL)"));
}

WebServer::WebServer(const Config& conf)
{
    _servers.reserve(conf.servers.size());
    for (size_t i = 0; i < conf.servers.size(); ++i)
        _servers.push_back(Server(conf.servers[i]));

    for (size_t i = 0; i < _servers.size(); ++i)
        _servers[i].setup();

    // Register all listener sockets in poll().
    for (size_t i = 0; i < _servers.size(); ++i) {
        const Sockets& socks = _servers[i].sockets();
        for (size_t j = 0; j < socks.size(); ++j) {
            const int fd = socks[j].get_socket();
            addListener(fd, i);
            setNonBlocking(fd);
        }
    }
}

void WebServer::addListener(int fd, size_t serverIndex)
{
    _listenerFds.insert(fd);
    _listenerToServerIndex[fd] = serverIndex;

    ::pollfd p;
    p.fd = fd;
    p.events = POLLIN;
    p.revents = 0;
    _pollfds.push_back(p);
}

void WebServer::addClient(int clientFd, int listenerFd, size_t serverIndex)
{
    ::pollfd p;
    p.fd = clientFd;
    p.events = POLLIN;
    p.revents = 0;
    _pollfds.push_back(p);

    _clients[clientFd] = ClientState(listenerFd, serverIndex);
}

void WebServer::closeAndRemove(size_t pollIndex)
{
    const int fd = _pollfds[pollIndex].fd;

    if (_listenerFds.count(fd) == 0) {
        (void)::close(fd);
        _clients.erase(fd);
    }

    // swap-remove
    if (pollIndex + 1 != _pollfds.size())
        _pollfds[pollIndex] = _pollfds[_pollfds.size() - 1];
    _pollfds.pop_back();
}

void WebServer::handleListenerReadable(int listenerPollIndex)
{
    const int listenerFd = _pollfds[listenerPollIndex].fd;
    const size_t serverIndex = _listenerToServerIndex[listenerFd];

    while (true) {
        const int clientFd = ::accept(listenerFd, 0, 0);
        if (clientFd < 0) {
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            throw std::runtime_error(syscallError("accept"));
        }

        setNonBlocking(clientFd);
        addClient(clientFd, listenerFd, serverIndex);
    }
}

bool WebServer::hasHeaderTerminator(const std::string& s)
{
    return s.find("\r\n\r\n") != std::string::npos || s.find("\n\n") != std::string::npos;
}

std::string WebServer::buildHelloResponse()
{
    const std::string body = "Hello from webserv\n";

    std::string resp;
    resp += "HTTP/1.1 200 OK\r\n";
    resp += "Content-Type: text/plain\r\n";
    resp += "Content-Length: ";

    // int -> string (C++98)
    {
        char tmp[32];
        ::snprintf(tmp, sizeof(tmp), "%lu", static_cast<unsigned long>(body.size()));
        resp += tmp;
    }

    resp += "\r\n";
    resp += "Connection: close\r\n";
    resp += "\r\n";
    resp += body;
    return resp;
}

bool WebServer::handleClientEvents(size_t clientPollIndex)
{
    const int fd = _pollfds[clientPollIndex].fd;

    std::map<int, ClientState>::iterator it = _clients.find(fd);
    if (it == _clients.end()) {
        closeAndRemove(clientPollIndex);
        return true;
    }

    ClientState& st = it->second;
    const short re = _pollfds[clientPollIndex].revents;

    if ((re & POLLIN) != 0) {
        char buf[4096];
        while (true) {
            const ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
            if (n > 0) {
                st.in.append(buf, static_cast<size_t>(n));
                continue;
            }
            if (n == 0) {
                closeAndRemove(clientPollIndex);
                return true;
            }

            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;

            closeAndRemove(clientPollIndex);
            return true;
        }

        if (!st.responded && hasHeaderTerminator(st.in)) {
            st.out = buildHelloResponse();
            st.responded = true;
        }
    }

    if ((re & POLLOUT) != 0 && !st.out.empty()) {
        int sendFlags = 0;
#ifdef MSG_NOSIGNAL
        sendFlags = MSG_NOSIGNAL;
#endif
        while (!st.out.empty()) {
            const ssize_t n = ::send(fd, st.out.data(), st.out.size(), sendFlags);
            if (n > 0) {
                st.out.erase(0, static_cast<size_t>(n));
                continue;
            }

            if (n < 0 && errno == EINTR)
                continue;
            if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
                break;

            closeAndRemove(clientPollIndex);
            return true;
        }

        if (st.responded && st.out.empty()) {
            closeAndRemove(clientPollIndex);
            return true;
        }
    }

    return false;
}

void WebServer::run()
{
    if (_pollfds.empty())
        throw std::runtime_error("No listen sockets configured");

    while (true) {
        // Refresh desired event masks (especially for clients that gained/emptied out buffers).
        for (size_t i = 0; i < _pollfds.size(); ++i) {
            const int fd = _pollfds[i].fd;
            _pollfds[i].revents = 0;

            if (_listenerFds.count(fd) != 0) {
                _pollfds[i].events = POLLIN;
                continue;
            }

            short ev = POLLIN;
            std::map<int, ClientState>::const_iterator it = _clients.find(fd);
            if (it != _clients.end() && !it->second.out.empty())
                ev |= POLLOUT;
            _pollfds[i].events = ev;
        }

        const int rc = ::poll(&_pollfds[0], _pollfds.size(), -1);
        if (rc < 0) {
            if (errno == EINTR)
                continue;
            throw std::runtime_error(syscallError("poll"));
        }

        for (size_t i = 0; i < _pollfds.size(); ) {
            const int fd = _pollfds[i].fd;
            const short re = _pollfds[i].revents;

            if (re == 0) {
                ++i;
                continue;
            }

            if ((re & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
                if (_listenerFds.count(fd) != 0)
                    throw std::runtime_error("Listener socket failed");
                closeAndRemove(i);
                continue;
            }

            if (_listenerFds.count(fd) != 0) {
                if ((re & POLLIN) != 0)
                    handleListenerReadable(static_cast<int>(i));
                ++i;
                continue;
            }

            if (handleClientEvents(i))
                continue;

            ++i;
        }
    }
}
