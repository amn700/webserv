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
    Socket sock = Socket(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY);

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


// Lexer.hpp (or inside a .cpp)
#include <vector>
#include <string>




TokenList lex(const std::vector<std::string>& lines)
{
    TokenList out;

    for (int ln = 0; ln < lines.size(); ++ln) {
        const std::string& s = lines[ln];
        const int lineNo = ln + 1;

        size_t i = 0;
        while (i < s.size()) {

            // 1) skip whitespace
            while (i < s.size() && isSpace(s[i]))
                ++i;
            if (i >= s.size())
                break;

            // 2) comment: ignore rest of the line
            if (s[i] == '#')
                break;

            // 3) single-character tokens
            if (s[i] == '{') { out.push_back(Token(TOK_LBRACE, "{", lineNo)); ++i; continue; }
            if (s[i] == '}') { out.push_back(Token(TOK_RBRACE, "}", lineNo)); ++i; continue; }
            if (s[i] == ';') { out.push_back(Token(TOK_SEMI,   ";", lineNo)); ++i; continue; }

            // 4) WORD token: read until whitespace/symbol/comment
            size_t start = i;
            while (i < s.size()) {
                char c = s[i];
                if (isSpace(c) || c == '{' || c == '}' || c == ';' || c == '#')
                    break;
                ++i;
            }
            out.push_back(Token(TOK_WORD, s.substr(start, i - start), lineNo));
        }
    }

    out.push_back(Token(TOK_EOF, "", static_cast<int>(lines.size() + 1)));
    return out;
}