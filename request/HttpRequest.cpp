#include "HttpRequest.hpp"

std::string parse_method(const std::string& str)
{
    size_t spacePos = str.find(' ');
    std::string firstWord = str.substr(0, spacePos);
    if(firstWord == "GET" || firstWord == "POST" || firstWord == "DELETE")
        return firstWord;
    else
        throw std::invalid_argument("ther is no method");
}

std::string parse_version(const std::string& str)
{
    size_t lastSpace = str.find_last_of(' ');
    std::string lastWord = str.substr(lastSpace + 1);
    return lastWord;
}

std::string pars_body(const std::string& str)
{
    if (str.find_first_not_of(" \t\n\r\f\v") == std::string::npos) 
        return "";
    std::string ret = str; 
    return ret;
}

std::string parse_path(const std::string& str)
{
    size_t firstSpace = str.find(' ');
    size_t secondSpace = str.find(' ', firstSpace + 1);
    std::string secondWord = str.substr(firstSpace + 1, secondSpace - firstSpace - 1);
    if(secondWord.find('?'))
    {
        size_t firstSepert = secondWord.find('?');
        std::string ret =secondWord.substr(0, firstSepert);
        return ret;
    }
    else
        return secondWord;
}
std::string trim_str(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}
std::map<std::string, std::string> pars_heders(std::vector<std::string> lines)
{
    int lastIndex = lines.size() - 1;
    std::map<std::string, std::string> ret;
    for (int i = 1; i <= lastIndex; i++)
    {
        size_t colon_pos = lines[i].find(':');
        std::string key = lines[i].substr(0, colon_pos);
        std::string value = lines[i].substr(colon_pos + 1);
        value = trim_str(value);
        ret.insert(std::make_pair(key, value));
    }
    return ret;
    // betwen firs and empty line
}

// std::map<std::string, std::string> pars_query(const std::string& str)
// {
    
//     // first line
// }

HttpRequest::HttpRequest(const std::string& raw_request)
{
    if (raw_request.empty())
        throw std::invalid_argument("String cannot be empty in MyObject constructor");
    std::vector<std::string> lines;
    std::stringstream ss(raw_request);
    std::string line;
    
    while (std::getline(ss, line)) 
    {
        lines.push_back(line);
    }
    this->method = parse_method(lines[0]);
        std::cout << this->method << std::endl;//
    this->path = parse_path(lines[0]);
        std::cout << this->path << std::endl;//
    this->version = parse_version(lines[0]);
        std::cout << this->version << std::endl;//
    this->body = pars_body(lines.back());
    if(!this->body.empty())
        std::cout << this->body << std::endl;//
    this->headers = pars_heders(lines);
    // this->query_params = pars_query(lines[0]);
}