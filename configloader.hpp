#ifndef CONFIGLOADER_HPP
#define CONFIGLOADER_HPP

#include "configtypes.hpp"
#include <string>
#include <stdexcept>

class ConfigLoader {
public:
    bool tryLoadFromFile(const std::string& path, Config& out, std::string* errorMessage) ;

private:
    Config loadFromFile(const std::string& path) ;
    std::vector<Token> tokinizer(const std::vector<std::string>& lines) ;
};

void expect(TokenList::const_iterator& it, TokenList::const_iterator end, TokenType type);

void expectWord(TokenList::const_iterator& it, TokenList::const_iterator end, const std::string& word);

ServerConfig::Listen parseListen(const std::string& s);

std::string to_lower(const std::string& s);

void parseLocationBlock(TokenList::const_iterator& it,TokenList::const_iterator end,ServerConfig& server,int serverWordLine);

std::string intToStr(int v);

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& message, int line)
        : std::runtime_error(message), _line(line) {}
    int line() const { return _line; }

private:
    int _line;
};

#endif
