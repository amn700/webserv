#include "webserv.hpp"
#include "configtypes.hpp"
#include "configloader.hpp"

int main (int argc, char ** argv)
{
    if (argc != 2)
        return std::cerr << "Error: invalid number of parameters\n Usage: ./server [config.conf]"<< std::endl, 1;
    // configuration file lexing/parsing
    ConfigLoader con;
    con.loadFromFile(argv[1]);
    
    // components setup
    // main loop
}
