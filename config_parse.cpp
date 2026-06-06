#include "configtypes.hpp"
#include "configloader.hpp"
// #include "webserv.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cctype>

std::string intToString(int v)
{
    std::ostringstream oss;
    oss << v;
    return oss.str();
}



std::string tokenTypeToString(TokenType t)
{
    switch (t) {
        case TOK_WORD:   return "TOK_WORD";
        case TOK_LBRACE: return "TOK_LBRACE";
        case TOK_RBRACE: return "TOK_RBRACE";
        case TOK_SEMI:   return "TOK_SEMI";
        case TOK_EOF:    return "TOK_EOF";
        default:         return "TOK_UNKNOWN";
    }
}

std::string tokenTypeToString_Litteral(TokenType t)
{
    switch (t) {
        case TOK_WORD:   return "word";
        case TOK_LBRACE: return "{";
        case TOK_RBRACE: return "}";
        case TOK_SEMI:   return ";";
        case TOK_EOF:    return "EOF";
        default:         return "TOK_UNKNOWN";
    }
}


void expect(TokenList::const_iterator& it,
            TokenList::const_iterator end,
            TokenType type)
{
    if (it == end)
        throw std::runtime_error("Parse error: expected " + tokenTypeToString_Litteral(type) + " but reached end of file");
    if (it->type != type)
        throw std::runtime_error("Parse error: expected " + tokenTypeToString_Litteral(type) + " at line " + intToString(it->line));
    ++it;
}

std::string to_lower(const std::string& s)
{
    std::string out = s;
    for (size_t i = 0; i < out.size(); ++i)
        out[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(out[i])));
    return out;
}


void expectWord(
    TokenList::const_iterator& it,
    TokenList::const_iterator end,
    const std::string& word
    )
{
    if (it == end || it->type != TOK_WORD || to_lower(it->text)!= word)
        throw std::runtime_error("Parse error: expected '" + word + "'" + " at line " + (it == end ? std::string("EOF") : intToString(it->line)));
    ++it;
}
void parse_validate(const std::vector<Token>& tokens, Config& conf)
{
    std::vector<Token>::const_iterator it = tokens.begin();

    while (it != tokens.end() && it->type != TOK_EOF)
    {
        ServerConfig server;
        DirectiveInfo dir;

        expectWord(it, tokens.end(), "server");
        int openBraceLine = (it != tokens.end() ? it->line : -1);
        expect(it, tokens.end(), TOK_LBRACE);

        while (it != tokens.end() && it->type != TOK_RBRACE && it->type != TOK_EOF)
        {
            if (it->type != TOK_WORD)
                throw std::runtime_error("Parse error: expected a directive at line " + intToString(it->line));
            
            if (to_lower(it->text) == "listen")
            {
                expectWord(it, tokens.end(), "listen");

                if (dir.content["listen"].seen && !dir.content["listen"].allowMultiple)
                    throw std::runtime_error("Parse error: duplicate directive 'listen' at line " + intToString(it->line));
                dir.content["listen"].seen = true;


                if (it == tokens.end() || it->type != TOK_WORD)
                    throw std::runtime_error("Parse error: listen expects host:port at line " + (it == tokens.end() ? std::string("EOF") : intToString(it->line)));

                server.listens.push_back(parseListenHostPort(it->text));


                expect(it, tokens.end(), TOK_WORD);
                expect(it, tokens.end(), TOK_SEMI);
            }

            else {
                throw std::runtime_error("Parse error: unknown directive '" + it->text + "' at line " + intToString(it->line));
            }
        }
        if (it == tokens.end() || it->type == TOK_EOF) {
            throw std::runtime_error(
                "Parse error: unexpected end of file; missing '}' to close server block opened at line " +
                intToString(openBraceLine)
            );
        }
        expect(it, tokens.end(), TOK_RBRACE);

        conf.servers.push_back(server);
    }
}


static std::vector<std::string> readFileLines(const std::string& path)
{
    std::ifstream in(path.c_str());
    if (!in.is_open())
        throw std::runtime_error("Cannot open config file: " + path);

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line))
        lines.push_back(line);

    if (in.fail() && !in.eof())
        throw std::runtime_error("Error while reading config file: " + path);

    return lines;
}

static bool isSpace(char c) { return (c == ' ' || c == '\t' || c == '\r'); }

std::vector<Token> tokinizer(const std::vector<std::string>& lines)
{
    TokenList out;

    for (size_t ln = 0; ln < lines.size(); ++ln) {
        const std::string& s = lines[ln];
        const int lineNo = static_cast<int>(ln + 1);

        size_t i = 0;
        while (i < s.size())
        {
            while (i < s.size() && isSpace(s[i]))
                ++i;
            if (i >= s.size())
                break;

            if (s[i] == '#')
                break;

            if (s[i] == '{') { out.push_back(Token(TOK_LBRACE, "{", lineNo)); ++i; continue; }
            if (s[i] == '}') { out.push_back(Token(TOK_RBRACE, "}", lineNo)); ++i; continue; }
            if (s[i] == ';') { out.push_back(Token(TOK_SEMI,   ";", lineNo)); ++i; continue; }

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

Config ConfigLoader:: loadFromFile(const std::string& path)
{
    Config conf;
    std::vector<std::string> lines = readFileLines(path);
    std::vector<Token> tokens = tokinizer(lines);
    parse_validate(tokens, conf);
    return conf;
}

// all possible server block content
// make sure the data bellow are correct
/*
    listen ::: mandatory, at least one; multiple allowed
        ex: listen ip:port;
    server_name ::: optional, only one allowed
        ex: server_name site1.local;
    root ::: mandatory, only one allowed
        ex: root /var/www/html/;
    client_max_body_size ::: optional, only one allowed; default 0 (use default)
        ex: client_max_body_size 10M;
    error_page ::: optional, multiple allowed; default empty
        ex: error_page 404 /404.html;
    location ::: optional, multiple allowed; default empty
        ex: location /upload/ { ... }
*/

//all possible location block content
/*
    prefix ::: mandatory, only one allowed;
        ex: /upload/ /cgi-bin/
    allowed_methods  ::: optional, only one allowed; default empty (use server/default)
        ex: GET POST DELETE
    redirect  ::: optional, only one allowed; default none
        ex: return 301 http://example.com/;
    root    ::: optional, only one allowed; default none (use server/default)
        ex: root /var/www/html/upload/;
    autoindex  ::: optional, only one allowed; default false
        ex: autoindex on;
    index  ::: optional, only one allowed; default empty
        ex: index index.html index.htm;
    upload  ::: optional, only one allowed; default none
        ex: upload /var/www/html/upload/;
    cgi  ::: optional, multiple allowed; default empty
        ex: cgi .php /usr/bin/php-cgi;
    return  ::: optional, only one allowed; default none
        ex: return 301 http://example.com/;
*/

