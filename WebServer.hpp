#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "configtypes.hpp"

#include <poll.h>

#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "Server.hpp"

class WebServer {
    public:
    WebServer(const Config& conf);
    
    // Blocking call: runs the poll() event loop until the process is terminated.
    void run();
    
    private:
    struct ClientState {
        std::string in;
        std::string out;
        int listenerFd;
        size_t serverIndex;
        bool responded;
        
        ClientState() : listenerFd(-1), serverIndex(0), responded(false) {}
        ClientState(int lfd, size_t sidx)
        : listenerFd(lfd), serverIndex(sidx), responded(false) {}
    };
    
    std::vector<Server> _servers;
    std::vector< ::pollfd > _pollfds;

    std::set<int> _listenerFds;
    std::map<int, size_t> _listenerToServerIndex;
    std::map<int, ClientState> _clients;

    static void setNonBlocking(int fd);

    void addListener(int fd, size_t serverIndex);
    void addClient(int clientFd, int listenerFd, size_t serverIndex);

    void closeAndRemove(size_t pollIndex);

    void handleListenerReadable(int listenerPollIndex);
    bool handleClientEvents(size_t clientPollIndex);

    static bool hasHeaderTerminator(const std::string& s);
    static std::string buildHelloResponse();
};

#endif
