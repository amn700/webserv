#include "ConfigLoader.hpp"
#include "configtypes.hpp"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cctype>

ServerConfig::Listen parseListenIPv4Port4(const std::string& s);

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& message, int line)
        : std::runtime_error(message), _line(line) {}
    int line() const { return _line; }

private:
    int _line;
};

static std::string intToString(int v)
{
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

static std::string tokenTypeToString_Litteral(TokenType t)
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


static void expect(TokenList::const_iterator& it,
            TokenList::const_iterator end,
            TokenType type)
{
    if (it == end)
        throw ParseError("Parse error: expected " + tokenTypeToString_Litteral(type) + " but reached end of file", -1);
    if (it->type != type)
        throw ParseError("Parse error: expected " + tokenTypeToString_Litteral(type), it->line);
    ++it;
}

static std::string to_lower(const std::string& s)
{
    std::string out = s;
    for (size_t i = 0; i < out.size(); ++i)
        out[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(out[i])));
    return out;
}


static void expectWord(
    TokenList::const_iterator& it,
    TokenList::const_iterator end,
    const std::string& word
    )
{
    if (it == end || it->type != TOK_WORD || to_lower(it->text)!= word)
        throw ParseError(
            "Parse error: expected '" + word + "'",
            (it == end ? -1 : it->line)
        );
    ++it;
}

static void parseLocationBlock(
    TokenList::const_iterator& it,
    TokenList::const_iterator end,
    ServerConfig& server,
    int serverWordLine
)
{
    expectWord(it, end, "location");

    if (it == end || it->type != TOK_WORD)
        throw ParseError(
            "Parse error: location expects a prefix",
            (it == end ? -1 : it->line)
        );

    ServerConfig::LocationConfig loc;
    LocationInfo locDir;

    loc.prefix = it->text;
    locDir.content["prefix"].seen = true;
    expect(it, end, TOK_WORD);

    const int locOpenBraceLine = (it != end ? it->line : -1);
    expect(it, end, TOK_LBRACE);

    while (it != end && it->type != TOK_RBRACE && it->type != TOK_EOF)
    {
        if (it->type != TOK_WORD)
            throw ParseError("Parse error: expected a directive inside location block", it->line);

        const std::string directive = to_lower(it->text);

        if (locDir.content.find(directive) != locDir.content.end())
        {
            if (!locDir.content[directive].allowMultiple && locDir.content[directive].seen)
                throw ParseError(
                    "Parse error: duplicate directive '" + directive + "' in location block",
                    it->line
                );
            locDir.content[directive].seen = true;
        }

        if (directive == "allowed_methods")
        {
            expectWord(it, end, "allowed_methods");

            if (it == end || it->type != TOK_WORD)
                throw ParseError(
                    "Parse error: allowed_methods expects at least one method",
                    (it == end ? -1 : it->line)
                );

            while (it != end && it->type == TOK_WORD)
            {
                std::string method = to_lower(it->text);
                if (method != "get" && method != "post" && method != "delete")
                    throw ParseError("Parse error: invalid HTTP method '" + it->text + "'", it->line);
                loc.methods.insert(method);
                expect(it, end, TOK_WORD);
            }
            expect(it, end, TOK_SEMI);
        }
        else if (directive == "return" || directive == "redirect")
        {
            // Accept both 'return' (as used by our sample config) and 'redirect' (legacy naming)
            expect(it, end, TOK_WORD);

            if (it == end || it->type != TOK_WORD)
                throw ParseError(
                    "Parse error: return expects a status code and a URL",
                    (it == end ? -1 : it->line)
                );

            int code = static_cast<int>(std::strtol(it->text.c_str(), NULL, 10));
            if (code <= 0)
                throw ParseError("Parse error: invalid return code '" + it->text + "'", it->line);
            expect(it, end, TOK_WORD);

            if (it == end || it->type != TOK_WORD)
                throw ParseError(
                    "Parse error: return expects a URL after the status code",
                    (it == end ? -1 : it->line)
                );

            loc.redirect.enabled = true;
            loc.redirect.code = code;
            loc.redirect.target = it->text;
            expect(it, end, TOK_WORD);
            expect(it, end, TOK_SEMI);
        }
        else if (directive == "root")
        {
            expectWord(it, end, "root");
            if (it == end || it->type != TOK_WORD)
                throw ParseError(
                    "Parse error: root expects a path inside location block",
                    (it == end ? -1 : it->line)
                );
            loc.root = it->text;
            expect(it, end, TOK_WORD);
            expect(it, end, TOK_SEMI);
        }
        else if (directive == "autoindex")
        {
            expectWord(it, end, "autoindex");
            if (it == end || it->type != TOK_WORD)
                throw ParseError(
                    "Parse error: autoindex expects 'on' or 'off'",
                    (it == end ? -1 : it->line)
                );
            const std::string v = to_lower(it->text);
            if (v != "on" && v != "off")
                throw ParseError("Parse error: autoindex expects 'on' or 'off'", it->line);
            loc.autoindex = (v == "on");
            expect(it, end, TOK_WORD);
            expect(it, end, TOK_SEMI);
        }
        else if (directive == "index")
        {
            expectWord(it, end, "index");
            if (it == end || it->type != TOK_WORD)
                throw ParseError(
                    "Parse error: index expects at least one filename",
                    (it == end ? -1 : it->line)
                );
            while (it != end && it->type == TOK_WORD)
            {
                loc.index.push_back(it->text);
                expect(it, end, TOK_WORD);
            }
            expect(it, end, TOK_SEMI);
        }
        else if (directive == "upload")
        {
            expectWord(it, end, "upload");
            if (it == end || it->type != TOK_WORD)
                throw ParseError(
                    "Parse error: upload expects 'on' or 'off'",
                    (it == end ? -1 : it->line)
                );
            const std::string v = to_lower(it->text);
            if (v != "on" && v != "off")
                throw ParseError("Parse error: upload expects 'on' or 'off'", it->line);
            loc.upload.enabled = (v == "on");
            expect(it, end, TOK_WORD);
            expect(it, end, TOK_SEMI);
        }
        else if (directive == "upload_dir")
        {
            expectWord(it, end, "upload_dir");
            if (it == end || it->type != TOK_WORD)
                throw ParseError(
                    "Parse error: upload_dir expects a path",
                    (it == end ? -1 : it->line)
                );
            loc.upload.enabled = true;
            loc.upload.dir = it->text;
            expect(it, end, TOK_WORD);
            expect(it, end, TOK_SEMI);
        }
        else if (directive == "cgi" || directive == "cgi_ext")
        {
            // Accept both 'cgi' (doc) and 'cgi_ext' (sample config)
            expect(it, end, TOK_WORD);

            if (it == end || it->type != TOK_WORD)
                throw ParseError(
                    "Parse error: cgi_ext expects an extension and a program path",
                    (it == end ? -1 : it->line)
                );
            const std::string ext = it->text;
            expect(it, end, TOK_WORD);

            if (it == end || it->type != TOK_WORD)
                throw ParseError(
                    "Parse error: cgi_ext expects a program path after the extension",
                    (it == end ? -1 : it->line)
                );
            const std::string prog = it->text;
            expect(it, end, TOK_WORD);

            loc.cgi[ext] = prog;
            expect(it, end, TOK_SEMI);
        }
        else
        {
            throw ParseError("Parse error: unknown directive '" + it->text + "' in location block", it->line);
        }
    }

    if (it == end || it->type == TOK_EOF)
    {
        throw ParseError(
            "Parse error: unexpected end of file; missing '}' to close location block opened here",
            (locOpenBraceLine > 0 ? locOpenBraceLine : serverWordLine)
        );
    }

    expect(it, end, TOK_RBRACE);

    server.locations.push_back(loc);
}

static void parse_validate(const std::vector<Token>& tokens, Config& conf)
{
    TokenList::const_iterator it = tokens.begin();

    while (it != tokens.end() && it->type != TOK_EOF)
    {
        ServerConfig server;
        DirectiveInfo dir;

        const int serverWordLine = (it != tokens.end() ? it->line : -1);

        expectWord(it, tokens.end(), "server");
        int openBraceLine = (it != tokens.end() ? it->line : -1);
        expect(it, tokens.end(), TOK_LBRACE);

        while (it != tokens.end() && it->type != TOK_RBRACE && it->type != TOK_EOF)
        {
            if (it->type != TOK_WORD)
                throw ParseError("Parse error: expected a directive", it->line);
            
            if (to_lower(it->text) == "listen")
            {
                expectWord(it, tokens.end(), "listen");

                if (dir.content["listen"].seen && !dir.content["listen"].allowMultiple)
                    throw ParseError("Parse error: duplicate directive 'listen'", it->line);
                dir.content["listen"].seen = true;


                if (it == tokens.end() || it->type != TOK_WORD)
                    throw ParseError(
                        "Parse error: listen expects ip:port",
                        (it == tokens.end() ? -1 : it->line)
                    );

                try {
                    server.listens.push_back(parseListenIPv4Port4(it->text));
                } catch (const std::exception& e) {
                    throw ParseError(std::string("Parse error: ") + e.what(), it->line);
                }


                expect(it, tokens.end(), TOK_WORD);
                expect(it, tokens.end(), TOK_SEMI);
            }
            else if (to_lower(it->text) == "server_name")
            {
                expectWord(it, tokens.end(), "server_name");

                if (dir.content["server_name"].seen && !dir.content["server_name"].allowMultiple)
                    throw ParseError("Parse error: duplicate directive 'server_name'", it->line);
                dir.content["server_name"].seen = true;

                if (it == tokens.end() || it->type != TOK_WORD)
                    throw ParseError(
                        "Parse error: server_name expects a name",
                        (it == tokens.end() ? -1 : it->line)
                    );

                server.server_name = it->text;
                expect(it, tokens.end(), TOK_WORD);
                expect(it, tokens.end(), TOK_SEMI);
            }
            else if (to_lower(it->text) == "root")
            {
                expectWord(it, tokens.end(), "root");

                if (dir.content["root"].seen && !dir.content["root"].allowMultiple)
                    throw ParseError("Parse error: duplicate directive 'root'", it->line);
                dir.content["root"].seen = true;

                if (it == tokens.end() || it->type != TOK_WORD)
                    throw ParseError(
                        "Parse error: root expects a path",
                        (it == tokens.end() ? -1 : it->line)
                    );

                server.root = it->text;
                expect(it, tokens.end(), TOK_WORD);
                expect(it, tokens.end(), TOK_SEMI);
            }
            else if (to_lower(it->text) == "client_max_body_size")
            {
                expectWord(it, tokens.end(), "client_max_body_size");

                if (dir.content["client_max_body_size"].seen && !dir.content["client_max_body_size"].allowMultiple)
                    throw ParseError("Parse error: duplicate directive 'client_max_body_size'", it->line);
                dir.content["client_max_body_size"].seen = true;

                if (it == tokens.end() || it->type != TOK_WORD)
                    throw ParseError(
                        "Parse error: client_max_body_size expects a size",
                        (it == tokens.end() ? -1 : it->line)
                    );

                try {
                    //  convert string to int
                    server.client_max_body_size = static_cast<size_t>(std::strtol(it->text.c_str(), NULL, 10));
                } catch (const std::exception& e) {
                    throw ParseError(std::string("Parse error: ") + e.what(), it->line);
                }

                expect(it, tokens.end(), TOK_WORD);
                expect(it, tokens.end(), TOK_SEMI);
            }
            else if (to_lower(it->text) == "error_page")
            {
                expectWord(it, tokens.end(), "error_page");

                if (!dir.content["error_page"].allowMultiple && dir.content["error_page"].seen)
                    throw ParseError("Parse error: duplicate directive 'error_page'", it->line);
                dir.content["error_page"].seen = true;

                if (it == tokens.end() || it->type != TOK_WORD)
                    throw ParseError(
                        "Parse error: error_page expects a code and a path",
                        (it == tokens.end() ? -1 : it->line)
                    );

                std::string codeStr = it->text;
                int code;
                try {
                    code = static_cast<int>(std::strtol(codeStr.c_str(), NULL, 10));
                } catch (const std::exception& e) {
                    throw ParseError(std::string("Parse error: ") + e.what(), it->line);
                }
                expect(it, tokens.end(), TOK_WORD);

                if (it == tokens.end() || it->type != TOK_WORD)
                    throw ParseError(
                        "Parse error: error_page expects a path after the code",
                        (it == tokens.end() ? -1 : it->line)
                    );

                server.error_pages[code] = it->text;
                expect(it, tokens.end(), TOK_WORD);
                expect(it, tokens.end(), TOK_SEMI);
            }
            // lets compact the location block into a function now
            else if (to_lower(it->text) == "location")
            {
                if (!dir.content["location"].allowMultiple && dir.content["location"].seen)
                    throw ParseError("Parse error: duplicate directive 'location'", it->line);
                dir.content["location"].seen = true;

                parseLocationBlock(it, tokens.end(), server, serverWordLine);
            }
            else {
                throw ParseError("Parse error: unknown directive '" + it->text + "'", it->line);
            }
        }
        if (it == tokens.end() || it->type == TOK_EOF) {
            throw ParseError(
                "Parse error: unexpected end of file; missing '}' to close server block opened here",
                (openBraceLine > 0 ? openBraceLine : serverWordLine)
            );
        }
        expect(it, tokens.end(), TOK_RBRACE);

        // enforce mandatory directives
        for (ServerContent::const_iterator di = dir.content.begin(); di != dir.content.end(); ++di) {
            if (di->second.isMandatory && !di->second.seen)
                throw ParseError(
                    "Parse error: missing mandatory directive '" + di->first + "' in server block",
                    (serverWordLine > 0 ? serverWordLine : 1)
                );
        }

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

// static std::vector<std::string> splitLines(const std::string& text)
// {
//     std::vector<std::string> lines;
//     std::string current;
//     for (size_t i = 0; i < text.size(); ++i) {
//         char c = text[i];
//         if (c == '\n') {
//             lines.push_back(current);
//             current.clear();
//         } else if (c != '\r') {
//             current.push_back(c);
//         }
//     }
//     // Keep the last line even if text doesn't end with '\n'
//     lines.push_back(current);
//     return lines;
// }

static std::string formatErrorWithContext(
    const std::string& sourceName,
    const std::vector<std::string>& lines,
    const ParseError& e)
{
    std::string out;
    out += sourceName;
    if (e.line() > 0)
        out += ":" + intToString(e.line());
    out += ": ";
    out += e.what();

    if (e.line() > 0 && static_cast<size_t>(e.line()) <= lines.size()) {
        out += "\n";
        out += lines[static_cast<size_t>(e.line() - 1)];
    }
    return out;
}

std::vector<Token> ConfigLoader::tokinizer(const std::vector<std::string>& lines)
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

Config ConfigLoader::loadFromFile(const std::string& path) 
{
    std::vector<std::string> lines = readFileLines(path);
    try {
        Config conf;
        std::vector<Token> tokens = tokinizer(lines);
        parse_validate(tokens, conf);
        return conf;
    } catch (const ParseError& e) {
        throw std::runtime_error(formatErrorWithContext(path, lines, e));
    }
}

// Config loadFromString(const std::string& text, const std::string& sourceName) 
// {
//     std::vector<std::string> lines = splitLines(text);
//     try {
//         Config conf;
//         std::vector<Token> tokens = tokinizer(lines);
//         parse_validate(tokens, conf);
//         return conf;
//     } catch (const ParseError& e) {
//         throw std::runtime_error(formatErrorWithContext(sourceName, lines, e));
//     }
// }

bool ConfigLoader::tryLoadFromFile(const std::string& path, Config& out, std::string* errorMessage) 
{
    try {
        out = loadFromFile(path);
        return true;
    } catch (const std::exception& e) {
        if (errorMessage)
            *errorMessage = e.what();
        return false;
    }
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
