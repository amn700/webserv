#include <iostream>
#include "configtypes.hpp"
#include "ConfigLoader.hpp"
#include "WebServer.hpp"

int main (int argc, char ** argv)
{
    if (argc != 2)
    {
        std::cerr << "Error: invalid number of parameters\n Usage: ./server [config.conf]" << std::endl;
        return 1;
    }

    Config conf;
    std::string errorMessage;
    if (!ConfigLoader().tryLoadFromFile(argv[1], conf, &errorMessage))
    {
        std::cerr << "Error: failed to load config file: " << argv[1] << ": " << errorMessage << std::endl;
        return 1;
    }

    try
    {
        WebServer server(conf);
        server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }


    return 0;
}
