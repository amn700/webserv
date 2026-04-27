#include "configtypes.hpp"
#include "configloader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

static std::vector<std::string> readFileLines(const std::string& path)
{
    std::ifstream in(path.c_str());
    if (!in.is_open())
        throw std::runtime_error("Cannot open config file: " + path);

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line))
        lines.push_back(line);

    if (in.fail() && !in.eof())
        throw std::runtime_error("Error while reading config file: " + path);

    return lines;
}

static std::vector<Token> tokinizer(std::vector<std::string> lines)
{
    TokenList tokens;
    for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it)
    {
        if (expect())
    }
    return tokens;
}

Config ConfigLoader:: loadFromFile(const std::string& path)
{
    Config conf;
    try
    {
        std::vector<std::string> lines = readFileLines(path);
        std::vector<Token> tokens = tokinizer(lines);
        for (size_t i = 0; i < lines.size(); i++)
            std::cout << tokens[i].type << " " << tokens[i].text << " " << tokens[i].line <<std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return conf;
}
