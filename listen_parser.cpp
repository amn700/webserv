#include <string>
#include <stdexcept>
#include <cctype>
#include <cstdlib>
#include "configtypes.hpp"
#include "configloader.hpp"

static int parseIntRange(const std::string& s, int minV, int maxV, int line)
{
    if (s.empty())
        throw ParseError("expected a number, got empty string", line);
    for (size_t i = 0; i < s.size(); ++i)
        if (!std::isdigit(static_cast<unsigned char>(s[i])))
            throw ParseError("expected a number, got: " + s, line);
    char* end;
    long v = std::strtol(s.c_str(), &end, 10);
    if (*end != '\0')
        throw ParseError("invalid number: " + s, line);
    if (v < minV || v > maxV)
        throw ParseError("number out of range [" + intToStr(minV) + ", "
                         + intToStr(maxV) + "]: " + s, line);
    return static_cast<int>(v);
}

ServerConfig::Listen parseListen(const std::string& s)
{
    size_t colon = s.find(':');
    if (colon == std::string::npos)
        throw std::runtime_error("listen: missing ':' (expected host:port)");
    if (s.find(':', colon + 1) != std::string::npos)
        throw std::runtime_error("listen: too many ':' characters");

    std::string host = s.substr(0, colon);
    std::string portStr = s.substr(colon + 1);

    if (host.empty())
        throw std::runtime_error("listen: empty host");
    if (portStr.empty())
        throw std::runtime_error("listen: empty port");

    int port = parseIntRange(portStr, 0, 65535, 0);

    ServerConfig::Listen l;
    l.host = host;
    l.port = port;
    return l;
}
