#ifndef SERVER_HPP
#define SERVER_HPP

#include "configtypes.hpp"
#include "Socket.hpp"

class Server {
public:
    explicit Server(const ServerConfig& config);

    // Creates/binds/listens on all configured listen sockets.
    // Throws std::runtime_error on failure.
    void setup();

    const ServerConfig& config() const;
    const Sockets& sockets() const;

private:
    ServerConfig _config;
    Sockets _sockets;
};

#endif
