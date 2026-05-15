#include "WebServer.hpp"

#include "Socket.hpp"

#include "request/HttpRequest.hpp"

#include <sys/socket.h>

#include <netdb.h>
#include <netinet/in.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <stdexcept>

static std::string peerToString(int fd)
{
    sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    ::memset(&ss, 0, sizeof(ss));

    if (::getpeername(fd, (sockaddr*)&ss, &slen) != 0)
        return "<unknown>";

    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];
    ::memset(host, 0, sizeof(host));
    ::memset(serv, 0, sizeof(serv));

    const int rc = ::getnameinfo((sockaddr*)&ss, slen,
                                 host, sizeof(host),
                                 serv, sizeof(serv),
                                 NI_NUMERICHOST | NI_NUMERICSERV);
    if (rc != 0)
        return "<unknown>";

    std::ostringstream out;
    out << host << ":" << serv;
    return out.str();
}

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
: _conf(conf)
{
    //Dedupe listeners by host:port and map
    // each listener fd to one or more server{} blocks (vhosts).
    std::map<std::string, int> keyToFd;

    for (size_t sidx = 0; sidx < conf.servers.size(); ++sidx) {
        const ServerConfig& sc = conf.servers[sidx];

        for (size_t lidx = 0; lidx < sc.listens.size(); ++lidx) {
            const ServerConfig::Listen& l = sc.listens[lidx];
            const std::string key = listenKey(l.host, l.port);

            int fd;
            // check if the key is already present in the map
            std::map<std::string, int>::iterator it = keyToFd.find(key);
            if (it == keyToFd.end()) {
                Socket* s = Socket::createListener(l.host, l.port, 128);
                fd = s->get_socket();
                _listeners.push_back(s);
                addListener(fd);
                setNonBlocking(fd);
                keyToFd[key] = fd;
            } else {
                fd = it->second;
            }

            _listenerToServerIndices[fd].push_back(sidx);
        }
    }
}

WebServer::~WebServer()
{
    for (std::map<int, ClientState>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        (void)::close(it->first);
    // Socket objects own listener fds — delete them so they close their fds.
    for (Sockets::iterator it = _listeners.begin(); it != _listeners.end(); ++it) {
        delete *it;
    }
    _listeners.clear();
    _listenerFds.clear();
}

void WebServer::addListener(int fd)
{
    _listenerFds.insert(fd);

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

    std::map<int, std::vector<size_t> >::const_iterator it = _listenerToServerIndices.find(listenerFd);
    if (it == _listenerToServerIndices.end() || it->second.empty())
        throw std::runtime_error("Internal error: listener has no associated server");

    // Default routing for new connections: first configured server{} for this listener.
    const size_t serverIndex = it->second[0];

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
            std::cerr << "[webserv] recv request from " << peerToString(fd)
                      << " (listenerFd=" << st.listenerFd
                      << ", serverIndex=" << st.serverIndex << ")\n";
            std::cerr << st.in << std::endl;

            try {
                const size_t idx = (st.serverIndex < _conf.servers.size()) ? st.serverIndex : 0;
                HttpRequest req(st.in, _conf.servers[idx]);
                std::cerr << "[webserv] parsed: method=" << req.method
                          << " path=" << req.path
                          << " status=" << req.status;
                if (!req.redirect_target.empty())
                    std::cerr << " redirect_target=" << req.redirect_target;
                std::cerr << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[webserv] parse error: " << e.what() << std::endl;
            }

            st.responded = true;
            closeAndRemove(clientPollIndex);
            return true;
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
