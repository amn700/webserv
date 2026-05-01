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


struct sockaddr {
	__uint8_t       sa_len;         // total length 
	sa_family_t     sa_family;      // [XSI] address family
	char            sa_data[14];    // [XSI] addr value (actually larger)
};

*/

int main() {
    Sockets all;
    
    for (;;)
        all.push_back() = Socket(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY);;
    // Socket sock = Socket(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY);
    all.push_back(Socket(sock));

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        perror("setsockopt");

    
    struct sockaddr_in addr = sock.get_address();
    if (bind(sock.get_socket(), (sockaddr*)&addr, sizeof(addr)) < 0)
        return perror("bind"),1;
    
    for (;;)
    {
        if (listen(sock.get_socket(), 10) < 0)
            return perror("listen"), 1;

        int client_fd = accept(sock.get_socket(), 0, 0);
        if (client_fd < 0)
            perror("accept");
        //////////////////////////////////////////////////
        char buf[4096];
        ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
        //////////////////////////////////////////////////
        write(1, buf, n);
        close(client_fd);
    }
    close(sock.get_socket());
    return 0;
}
