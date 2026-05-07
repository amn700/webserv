// #include "webserv.hpp"
#include "HttpRequest.hpp"



int main (int argc, char ** argv)
{
    if (argc != 2)
        return std::cerr << "Error: invalid number of parameters\n Usage: ./server [config.conf]"<< std::endl, 1;

    try
    {
        std::string raw_string = argv[1];
        HttpRequest request(raw_string);
        //  = RequestParser::parse(raw_string);

    }
    catch(const std::invalid_argument& e)
    {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
    
}
