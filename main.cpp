#include "webserv.hpp"
#include "HttpRequest.hpp"

int main (int argc, char ** argv)
{
    if (argc != 2)
        return std::cerr << "Error: invalid number of parameters\n Usage: ./server [config.conf]"<< std::endl, 1;
    // configuration file lexing/parsing
    // components setup
    // main loop
    // 1. Your parser creates the object


// // 2. You pass it to response person
// ResponseBuilder response_builder;
// std::string http_response = response_builder.build(request);

// // 3. Your socket teammate sends it back
// send(socket_fd, http_response.c_str(), http_response.size(), 0);
}
