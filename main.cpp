#include "webserv.hpp"

#include "HttpRequest.hpp"
#include "configtypes.hpp"
#include "configloader.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>


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
    Sockets all;

    for (size_t si = 0; si < here.servers.size(); ++si) {
        for (size_t li = 0; li < here.servers[si].listens.size(); ++li) {

            const std::string& host = here.servers[si].listens[li].host;
            int port = here.servers[si].listens[li].port;

            in_addr addr;
            if (inet_pton(AF_INET, host.c_str(), &addr) != 1) {
                std::cerr << "Invalid IPv4 address in listen: " << host << std::endl;
                return 1;
            }

            all.push_back(Socket(AF_INET, SOCK_STREAM, 0, port, addr.s_addr));
        }
    }
    // components setup
    // main loop
    // 1. Your parser creates the object


// // 2. You pass it to response person
// ResponseBuilder response_builder;
// std::string http_response = response_builder.build(request);

// // 3. Your socket teammate sends it back
// send(socket_fd, http_response.c_str(), http_response.size(), 0);
}
