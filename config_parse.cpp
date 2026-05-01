#include "configtypes.hpp"
#include "configloader.hpp"
#include "webserv.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cctype>

Listen parseListenIPv4Port4(const std::string& s);



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
    if (it == end || it->type != type)
        throw std::runtime_error("Parse error: expected " + tokenTypeToString_Litteral(type) + " at line " + std::to_string(it->line - 1));
    it++;
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
        throw std::runtime_error("Parse error: expected '" + word + "'" + " at line " + std::to_string(it->line));
    it++;
}
void parse_validate(const std::vector<Token>& tokens, Config& conf)
{
    std::vector<Token>::const_iterator it = tokens.begin();

    while (it != tokens.end() && it->type != TOK_EOF)
    {
        ServerConfig server;
        DirectiveInfo dir;

        expectWord(it, tokens.end(), "server");
        expect(it, tokens.end(), TOK_LBRACE);

        while (it != tokens.end())
        {
            if (it == tokens.end() || it->type == TOK_EOF)
                throw std::runtime_error(
                    "Parse error: unexpected end of file (missing '}' to close server block), "
                    "last seen at line " + std::to_string(lastLine));
            if (it->type == TOK_RBRACE)
                break;
            if (it->type != TOK_WORD)
                throw std::runtime_error("Parse error: expected a directive at line " + std::to_string(it->line));
            
            if (to_lower(it->text) == "listen")
            {
                expectWord(it, tokens.end(), "listen");

                if (dir.content["listen"].seen && !dir.content["listen"].allowMultiple)
                    throw std::runtime_error("Parse error: duplicate directive 'listen' at line " + std::to_string(it->line));
                dir.content["listen"].seen = true;


                if (it == tokens.end() || it->type != TOK_WORD)
                    throw std::runtime_error("Parse error: listen expects ip:port at line " + std::to_string(it->line));

                server.listens.push_back(parseListenIPv4Port4(it->text));


                expect(it, tokens.end(), TOK_WORD);
                expect(it, tokens.end(), TOK_SEMI);
            }

            else {
                throw std::runtime_error("Parse error: unknown directive '" + it->text +
                                         "' at line " + std::to_string(it->line));
            }
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

std::vector<Token> tokinizer(std::vector<std::string> lines)
{
    TokenList out;

    for (int ln = 0; ln < lines.size(); ++ln) {
        const std::string& s = lines[ln];
        const int lineNo = ln + 1;

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
    try
    {
        std::vector<std::string> lines = readFileLines(path);
        // for (size_t i = 0; i < lines.size(); i++)
        //     std::cout << lines[i] << std::endl;
        std::vector<Token> tokens = tokinizer(lines);
        // for (size_t i = 0; i < tokens.size(); i++)
        //     std::cout << tokenTypeToString(tokens[i].type) << " " << tokens[i].text << " " << tokens[i].line <<std::endl;
        parse_validate(tokens, conf);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
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

