#include "configtypes.hpp"
#include "configloader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cctype>

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


void expect(TokenList::const_iterator& it,
            TokenList::const_iterator end,
            TokenType type)
{
    if (it == end || it->type != type)
        throw std::runtime_error("Parse error: expected different token");
    ++it;
}

std::string to_lower(const std::string& s)
{
    std::string out = s;
    for (size_t i = 0; i < out.size(); ++i) {
        out[i] = static_cast<char>(
            std::tolower(static_cast<unsigned char>(out[i]))
        );
    }
    return out;
}


void expectWord(
    TokenList::const_iterator& it,
    TokenList::const_iterator end,
    const std::string& word
    )
{
    if (it == end || it->type != TOK_WORD || to_lower(it->text)!= word)
        throw std::runtime_error("Parse error: expected '" + word + "'");
    ++it;
}

void parse_validate(std::vector<Token> tokens, Config conf)
{
    std::vector<Token>::const_iterator it = tokens.begin();
    // for (it = tokens.begin(); it != tokens.end(); ++it)
    //     {}
        expectWord(it, tokens.end(), "server");
        expect(it, tokens.end(), TOK_LBRACE);
        
    // }
}

Config ConfigLoader:: loadFromFile(const std::string& path)
{
    Config conf;
    try
    {
        std::vector<std::string> lines = readFileLines(path);
        std::vector<Token> tokens = tokinizer(lines);
        // for (size_t i = 0; i < lines.size(); i++)
        //     std::cout << tokenTypeToString(tokens[i].type) << " " << tokens[i].text << " " << tokens[i].line <<std::endl;
        parse_validate(tokens, conf);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return conf;
}
