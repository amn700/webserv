#ifndef CONFIGLOADER_HPP
#define CONFIGLOADER_HPP

#include "configtypes.hpp"
#include <string>

class ConfigLoader {
public:
    // Useful for tests and for loading config from memory.
    // Config loadFromString(const std::string& text, const std::string& sourceName) ;

    // Non-throwing convenience wrapper.
    // Returns true on success; false on error and fills errorMessage if provided.
    bool tryLoadFromFile(const std::string& path, Config& out, std::string* errorMessage) ;

private:
    Config loadFromFile(const std::string& path) ;
    std::vector<Token> tokinizer(const std::vector<std::string>& lines) ;
    // Throws std::runtime_error/ParseError-style messages on failure.

    // - tokenize()
    // - parseConfig()
    // - parseServer()
    // - parseLocation()
    // - validate()
};


#endif
