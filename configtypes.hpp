#ifndef CONFIGTYPES_HPP
#define CONFIGTYPES_HPP

#include <string>
#include <vector>
#include <map>
#include <set>


struct ServerConfig {
    struct Listen {
        std::string host;
        int port;
    };
    std::vector<Listen> listens;

    std::string server_name;
    std::string root;
    size_t client_max_body_size;
    std::map<int, std::string> error_pages;
    struct LocationConfig {
        struct Redirect {
            bool enabled;
            int code;
            std::string target;
            Redirect() : enabled(false), code(0) {}
        };
        struct UploadConfig {
            bool enabled;
            std::string dir;
            UploadConfig() : enabled(false) {}
        };
        typedef std::set<std::string> MethodSet;
        typedef std::map<std::string, std::string> CgiMap; 
        
        
        std::string prefix;
        MethodSet methods;
        Redirect redirect;
        std::string root;
        bool autoindex;
        std::vector<std::string> index;
        UploadConfig upload;
        CgiMap cgi;

        LocationConfig() : autoindex(false) {}
    };

    std::vector<LocationConfig> locations;

    ServerConfig() : client_max_body_size(0) {}
};

struct Config {
    std::vector<ServerConfig> servers;
};

enum TokenType {
    TOK_WORD,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_SEMI,
    TOK_EOF
};

struct Token {
    TokenType type;
    std::string text;
    int line;

    Token() : type(TOK_EOF), text(""), line(1) {}
    Token(TokenType t, const std::string& s, int ln) : type(t), text(s), line(ln) {}
};
typedef std::vector<Token> TokenList;




struct DirectiveProps {
    bool seen;
    bool isMandatory;
    bool allowMultiple;

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
        content["root"]                 = DirectiveProps(false, true,  false);
        content["client_max_body_size"] = DirectiveProps(false, false, false);
        content["error_page"]           = DirectiveProps(false, false, true);
        content["location"]             = DirectiveProps(false, false, true);
    }
};


struct LocationProps {
    bool seen;
    bool isMandatory;
    bool allowMultiple;

    LocationProps() : seen(false), isMandatory(false), allowMultiple(false) {}
    LocationProps(bool s, bool m, bool multi)
        : seen(s), isMandatory(m), allowMultiple(multi) {}
};


typedef std::map<std::string, LocationProps> LocationContent;

struct LocationInfo {
    LocationContent content;

    LocationInfo() {
        content["prefix"]          = LocationProps(false, true,  false);
        content["allowed_methods"] = LocationProps(false, false, false);
        content["redirect"]        = LocationProps(false, false, false);
        content["root"]            = LocationProps(false, false, false);
        content["autoindex"]       = LocationProps(false, false, false);
        content["index"]           = LocationProps(false, false, false);
        content["upload"]          = LocationProps(false, false, false);
        content["cgi"]             = LocationProps(false, false, true);
        content["return"]          = LocationProps(false, false, false);
    }
};

#endif