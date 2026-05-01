#include "webserv.hpp"
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

    Config here = ConfigLoader().loadFromFile(argv[1]);
    Sockets all;

    for (size_t si = 0; si < here.servers.size(); ++si) {
        for (size_t li = 0; li < here.servers[si].listens.size(); ++li) {

            const std::string& host = here.servers[si].listens[li].host;
            int port = here.servers[si].listens[li].port;

            in_addr addr;
            if (inet_pton(AF_INET, host.c_str(), &addr) != 1) {
            //     throw std::runtime_error("Invalid IPv4 address in listen: " + host);
            // }

            all.push_back(Socket(AF_INET, SOCK_STREAM, 0, port, addr.s_addr));
        }
    }
    // components setup
    // main loop
}
