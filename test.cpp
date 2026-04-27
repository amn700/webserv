#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "webserv.hpp"
#include <errno.h>


/*

struct sockaddr_in {
	__uint8_t       sin_len;
	sa_family_t     sin_family;
	in_port_t       sin_port;
	struct  in_addr sin_addr;
	char            sin_zero[8];
};

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
int main() {
    Socket sock = Socket(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY);
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        return perror("socket"), 1;

    struct sockaddr_in addr = sock.get_address();
    if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
        return perror("bind"),1;

    if (listen(listen_fd, 10) < 0)
        return perror("listen"), 1;

    int client_fd = accept(listen_fd, 0, 0);
    if (client_fd < 0)
        perror("accept");

    write(client_fd, "chfiha\n", 8);
   
    close(client_fd);
    close(listen_fd);
    return 0;
}