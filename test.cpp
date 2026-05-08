#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "webserv.hpp"
#include <errno.h>
#include <cstdlib>
#include "request/HttpRequest.hpp"

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

static MethodSet make_methods(const char* a = 0, const char* b = 0, const char* c = 0)
{
    MethodSet s;
    if (a) s.insert(a);
    if (b) s.insert(b);
    if (c) s.insert(c);
    return s;
}

// static void print_result(const std::string& title, const HttpRequest& req)
// {
//     std::cout << "==== " << title << " ====\n";
//     std::cout << "method: " << req.method << "\n";
//     std::cout << "path:   " << req.path << "\n";
//     std::cout << "status: " << req.status << "\n";
//     std::cout << "redirect_target: " << req.redirect_target << "\n";
//     std::cout << "\n";
// }


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
     recv(client_fd, buf, sizeof(buf), 0);
    // if (n > 0)
    //     (void)write(1, buf, static_cast<size_t>(n));


    ServerConfig serv;
    serv.root = "."; // create ./www and some files inside for testing

    // location /old -> redirect to /new with 301
    LocationConfig loc_old;
    loc_old.prefix = "/old";
    loc_old.redirect.enabled = true;
    loc_old.redirect.code = 301;
    loc_old.redirect.target = "/new";
    serv.locations.push_back(loc_old);

    // location /images/ -> normal files, only GET allowed
    LocationConfig loc_images;
    loc_images.prefix = "/images/";
    loc_images.methods = make_methods("GET");
    serv.locations.push_back(loc_images);

    // location / -> catch-all (optional but recommended)
    LocationConfig loc_root;
    loc_root.prefix = "/";
    loc_root.methods = make_methods("GET", "POST", "DELETE"); // allow all 3 for testing
    serv.locations.push_back(loc_root);

    HttpRequest req = HttpRequest(buf, serv);
    




    close(client_fd);
    return 0;
}
