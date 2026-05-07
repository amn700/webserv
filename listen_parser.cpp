#include <string>
#include <stdexcept>
#include <cctype>
#include <cstdlib>
#include "configtypes.hpp"

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

Listen parseListenHostPort(const std::string& s)
{
    if (s.empty())
        throw std::runtime_error("listen: empty value (expected host:port)");

    std::string host;
    std::string portStr;

    // Support bracketed IPv6: [::1]:8080
    if (s[0] == '[') {
        size_t close = s.find(']');
        if (close == std::string::npos)
            throw std::runtime_error("listen: missing ']' (expected [addr]:port)");
        if (close + 1 >= s.size() || s[close + 1] != ':')
            throw std::runtime_error("listen: missing ':' after ']' (expected [addr]:port)");

        host = s.substr(1, close - 1);
        portStr = s.substr(close + 2);
        if (host.empty())
            throw std::runtime_error("listen: empty address inside '[]'");
    } else {
        size_t colon = s.find(':');
        if (colon == std::string::npos)
            throw std::runtime_error("listen: missing ':' (expected host:port)");

        if (s.find(':', colon + 1) != std::string::npos)
            throw std::runtime_error("listen: too many ':' characters (use [addr]:port for IPv6)");

        host = s.substr(0, colon);
        portStr = s.substr(colon + 1);

        // Common shorthand: ":8080" or "*:8080" means bind all interfaces.
        if (host.empty() || host == "*")
            host = "0.0.0.0";
    }

    int port = parseIntRange(portStr, 0, 65535);

    Listen l;
    l.host = host;
    l.port = port;
    return l;
}

// Backwards-compatible name kept for the rest of the codebase.
// Despite the name, it now accepts either a numeric IPv4 string or a DNS hostname.
Listen parseListenIPv4Port4(const std::string& s)
{
    return parseListenHostPort(s);
}
