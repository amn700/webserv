#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "webserv.hpp"
#include <errno.h>
#include <cstdlib>


/*

struct sockaddr_in {
	__uint8_t       sin_len;
	sa_family_t     sin_family;
	in_port_t       sin_port;
	struct  in_addr sin_addr;
	char            sin_zero[8];
};


struct sockaddr {
	__uint8_t       sa_len;         // total length 
	sa_family_t     sa_family;      // [XSI] address family
	char            sa_data[14];    // [XSI] addr value (actually larger)
};

*/

#ifdef WEBSERV_TEST
int main() {
    Socket sock(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY);

    int opt = 1;
    if (setsockopt(sock.get_socket(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        return perror("setsockopt"), 1;

    struct sockaddr_in addr = sock.get_address();
    if (bind(sock.get_socket(), (sockaddr*)&addr, sizeof(addr)) < 0)
        return perror("bind"), 1;

    if (listen(sock.get_socket(), 10) < 0)
        return perror("listen"), 1;

    int client_fd = accept(sock.get_socket(), 0, 0);
    if (client_fd < 0)
        return perror("accept"), 1;

    char buf[4096];
    ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
    if (n > 0)
        (void)write(1, buf, static_cast<size_t>(n));

    close(client_fd);
    return 0;
}
#endif
