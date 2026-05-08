#ifndef CONFIGLOADER_HPP
#define CONFIGLOADER_HPP

#include "configtypes.hpp"
#include <string>
#include <vector>

class ConfigLoader {
public:
    Config loadFromFile(const std::string& path);

private:
    // - tokenize()
    // - parseConfig()
    // - parseServer()
    // - parseLocation()
    // - validate()
};


#endif