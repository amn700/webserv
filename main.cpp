#include "webserv.hpp"

#include "HttpRequest.hpp"
#include "configtypes.hpp"
#include "configloader.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>

static in_addr resolveIPv4(const std::string& host)
{
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;      // IPv4 only (Socket uses sockaddr_in)
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* res = NULL;
    int rc = ::getaddrinfo(host.c_str(), NULL, &hints, &res);
    if (rc != 0 || res == NULL)
        throw std::runtime_error(std::string("getaddrinfo failed for '") + host + "': " + ::gai_strerror(rc));

    struct sockaddr_in* sin = reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
    in_addr out = sin->sin_addr;
    ::freeaddrinfo(res);
    return out;
}


int main (int argc, char ** argv)
{
    if (argc != 2)
        return std::cerr << "Error: invalid number of parameters\n Usage: ./server [config.conf]"<< std::endl, 1;
    // configuration file lexing/parsing

    Config here;
    try {
        here = ConfigLoader().loadFromFile(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    std::cout << here.servers[0].listens[0].host << ":" << here.servers[0].listens[0].port << std::endl;

    Sockets all;
    for (size_t si = 0; si < here.servers.size(); ++si) {
        for (size_t li = 0; li < here.servers[si].listens.size(); ++li) {

            const std::string& host = here.servers[si].listens[li].host;
            int port = here.servers[si].listens[li].port;

            in_addr addr = resolveIPv4(host);
            all.push_back(Socket(AF_INET, SOCK_STREAM, 0, port, addr.s_addr));
        }
    }
}
