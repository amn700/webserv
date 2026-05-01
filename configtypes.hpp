#ifndef CONFIGTYPES_HPP
#define CONFIGTYPES_HPP

#include <string>
#include <vector>
#include <map>
#include <set>

struct Listen {
    std::string host; // "127.0.0.1" or "0.0.0.0" l ip dial server
    int port;         // 1 .. 65535 ..... ex: 8080 port dial server
};

struct Redirect {
    bool enabled;
    int code;               // 301/302/...
    std::string target;     // "/new" or full URL
    Redirect() : enabled(false), code(0) {}
};

struct UploadConfig {
    bool enabled;
    std::string dir;
    UploadConfig() : enabled(false) {}
};

enum TokenType {
    TOK_WORD,     // identifiers + values: listen, 127.0.0.1, /upload/, on, 404, etc.
    TOK_LBRACE,   // {
    TOK_RBRACE,   // }
    TOK_SEMI,     // ;
    TOK_EOF       // end of input (optional but very convenient for parser)
};

struct Token {
    TokenType type;
    std::string text; // only used for TOK_WORD (and maybe TOK_EOF = "")
    int line;

    Token() : type(TOK_EOF), text(""), line(1) {}
    Token(TokenType t, const std::string& s, int ln) : type(t), text(s), line(ln) {}
};
typedef std::vector<Token> TokenList;

typedef std::set<std::string> MethodSet;
typedef std::map<std::string, std::string> CgiMap; // ".py" -> "/usr/bin/python3" ...

struct LocationConfig {
    std::string prefix;                 // "/upload/" "/cgi-bin/" etc.
    MethodSet methods;                  // allowed methods; empty => server/default
    Redirect redirect;                  // optional
    std::string root;                   // optional override
    bool autoindex;                     // default false
    std::vector<std::string> index;     // ["index.html", ...]
    UploadConfig upload;                // optional
    CgiMap cgi;                         // optional

    LocationConfig() : autoindex(false) {}
};

struct ServerConfig {
    std::vector<Listen> listens;                 // interface:port pairs
    std::string server_name;                     // optional
    std::string root;                            // required
    size_t client_max_body_size;                 // bytes; 0 => use default
    std::map<int, std::string> error_pages;      // 404 -> "path"

    std::vector<LocationConfig> locations;

    ServerConfig() : client_max_body_size(0) {}
};

struct Config {
    std::vector<ServerConfig> servers;
};

// directive name, is_set, is_mandatory, is_multiple_allowed
struct DirectiveProps {
    bool seen;           // have we encountered it (for the current server block)
    bool isMandatory;    // must exist?
    bool allowMultiple;  // can appear more than once?

    DirectiveProps() : seen(false), isMandatory(false), allowMultiple(false) {}
    DirectiveProps(bool s, bool m, bool multi)
        : seen(s), isMandatory(m), allowMultiple(multi) {}
};

typedef std::map<std::string, DirectiveProps> ServerContent;

struct DirectiveInfo {
    ServerContent content;

    DirectiveInfo() {
        content["listen"]               = DirectiveProps(false, true,  true);
        content["server_name"]          = DirectiveProps(false, false, false);
        content["root"]                 = DirectiveProps(false, false,  false); // maybe maybe
        content["client_max_body_size"] = DirectiveProps(false, false, false);
        content["error_page"]           = DirectiveProps(false, false, true);
        content["location"]             = DirectiveProps(false, false, true);
    }
};

#endif