#include <string>
#include <stdexcept>
#include <cctype>
#include <cstdlib>
#include "configtypes.hpp"

static std::string toLower(const std::string& s)
{
    std::string out = s;
    for (size_t i = 0; i < out.size(); ++i)
        out[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(out[i])));
    return out;
}
static int parseIntRange(const std::string& s, int minV, int maxV)
{
    if (s.empty())
        throw std::runtime_error("empty number");

    for (size_t i = 0; i < s.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(s[i])))
            throw std::runtime_error("non-digit in number: " + s);
    }

    char* end = 0;
    // errno = 0;
    long v = std::strtol(s.c_str(), &end, 10);
    if (/*errno == ERANGE|| */ end == s.c_str() || *end != '\0')
        throw std::runtime_error("bad number: " + s);

    if (v < minV || v > maxV)
        throw std::runtime_error("number out of range: " + s);

    return static_cast<int>(v);
}

static bool isValidIPv4(const std::string& ip)
{
    int parts = 0;
    size_t start = 0;

    while (true) {
        size_t dot = ip.find('.', start);
        std::string part = (dot == std::string::npos)
            ? ip.substr(start)
            : ip.substr(start, dot - start);

        if (part.empty()) return false;

        try {
            (void)parseIntRange(part, 0, 255);
        } catch (...) {
            return false;
        }

        ++parts;
        if (dot == std::string::npos) break;
        start = dot + 1;
    }

    return parts == 4;
}

Listen parseListenIPv4Port4(const std::string& s)
{
    size_t colon = s.find(':');
    if (colon == std::string::npos)
        throw std::runtime_error("listen: missing ':' (expected ip:port)");

    if (s.find(':', colon + 1) != std::string::npos)
        throw std::runtime_error("listen: too many ':' characters");

    std::string ip = s.substr(0, colon);
    std::string portStr = s.substr(colon + 1);

    if (toLower(ip) == "localhost")
        ip = "127.0.0.1";

    if (!isValidIPv4(ip))
        throw std::runtime_error("listen: invalid IPv4 address: " + ip);

    int port = parseIntRange(portStr, 0, 65535);

    Listen l;
    l.host = ip;
    l.port = port;
    return l;
}
