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

std::string pars_body(std::vector<std::string>lines)
{
    unsigned int i;
    for(i=1;lines.size();i++)
    {
        if(lines[i].find_first_not_of(" \t\n\r\f\v") == std::string::npos)
            break;
    }
    i=i+1;
    if (i==lines.size())
        return "";
    std::string ret;
    while (i < lines.size())
    {
        ret += lines[i];
        if (i != lines.size() - 1)
            ret += "\n";
        i++;
    }
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
    if (start == std::string::npos)
        return "";
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

std::map<std::string, std::string> pars_heders(std::vector<std::string> lines)
{
    std::map<std::string, std::string> ret;
    for (unsigned int i = 1; i < lines.size(); i++)
    {
        if(lines[i].find_first_not_of(" \t\n\r\f\v") == std::string::npos)
            break;
        size_t colon_pos = lines[i].find(':');
        std::string key = lines[i].substr(0, colon_pos);
        std::string value = lines[i].substr(colon_pos + 1);
        value = trim_str(value);
        ret.insert(std::make_pair(key, value));
    }
    return ret;
}

std::vector<std::string> split(const std::string& str, char delimiter)
{
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter))
        result.push_back(token);
    
    return result;
}

std::map<std::string, std::string> pars_query(const std::string& str)
{
    std::map<std::string, std::string> ret;
    size_t lastSpace = str.find_last_of('?');
    if(lastSpace  == std::string::npos)
        return ret;
    std::string lastWord = str.substr(lastSpace + 1);
    size_t spacePos = lastWord.find(' ');
    std::string theword = lastWord.substr(0, spacePos);
    std::vector<std::string> result = split(theword , '&');
    for(unsigned int i = 0; i < result.size();i++)
    {
        size_t colon_pos = result[i].find('=');
        std::string key;
        std::string value;
        if(colon_pos == std::string::npos)
        {
            key = result[i];
            value = "";
        }
        else
        {
            key = result[i].substr(0, colon_pos);
            value = result[i].substr(colon_pos + 1);
        }
        ret.insert(std::make_pair(key, value));
    }
    return ret;
}

const LocationConfig* best_match_location(const std::string& path, const ServerConfig& serv)
{
    const LocationConfig* best = NULL;
    size_t bestLen = 0;
    for (size_t i = 0; i < serv.locations.size(); ++i)
    {
        const std::string& prefix = serv.locations[i].prefix;
        if (path.rfind(prefix, 0) == 0)
        {
            if (prefix.size() > bestLen)
            {
                bestLen = prefix.size();
                best = &serv.locations[i];
            }
        }
    }
    return best;
}

bool method_allowed(const std::string& method, const LocationConfig* loc)
{
    if (!loc)
        return true; 
    if (loc->methods.empty())
        return true;
    return loc->methods.count(method) != 0;
}

int check_path_get(const std::string& fs_path)
{
    struct stat st;

    if (stat(fs_path.c_str(), &st) != 0)
    {
        if (errno == ENOENT || errno == ENOTDIR)
            return 404;
        if (errno == EACCES || errno == EPERM)
            return 403;//u cant open som of the directorys tht the fille is in to 
        return 500;
    }
    // If it's a directory, handle separately (index/autoindex logic)
    if (S_ISDIR(st.st_mode))//u write a directory insted of file 
        return 403; // or special handling; don't just return 200
    if (access(fs_path.c_str(), R_OK) != 0) //permition to reed 
    {
        if (errno == EACCES || errno == EPERM)
            return 403;//u cant open the file itself
        return 500;
    }
    return 200;
}

int HttpRequest::validate_request(const ServerConfig& serv)
{
    const LocationConfig* loc = best_match_location(this->path, serv);

    // 1) redirect first
    if (loc && loc->redirect.enabled) 
    {
        this->redirect_target = loc->redirect.target;   // if you store it here
        return loc->redirect.code;                      // 301/302...
    }

    // 2) method check
    if (!method_allowed(this->method, loc))
        return 405;

    // 3) build filesystem path (root + URI)
    std::string root = serv.root;
    if (loc && !loc->root.empty())
        root = loc->root;

    std::string fs_path = root + this->path;

    // // 4) file checks
    // if (!path_exists(fs_path))
    //     return 404;
    // if (!has_permission(fs_path)) // R_OK for GET, etc.
    //     return 403;

    return check_path_get(fs_path);
}

HttpRequest::HttpRequest(const std::string& raw_request,ServerConfig serv)
{
    // std::cout<<"PARSING"<<std::endl;
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
        // std::cout << this->method << std::endl;//

    this->path = parse_path(lines[0]);
        // std::cout << this->path << std::endl;//

    this->version = parse_version(lines[0]);
        // std::cout << this->version << std::endl;//

    this->body = pars_body(lines);//
    // if(!this->body.empty())
        // std::cout <<"--"<< this->body<<"--" << std::endl;//

    this->headers = pars_heders(lines);
    // std::cout << "Key: "  << "=== Value: "  << std::endl;
    // for (std::map<std::string, std::string>::iterator it = this->headers.begin();it != this->headers.end(); ++it)
    // std::cout << it->first <<"    " << it->second << std::endl;

    this->query_params = pars_query(lines[0]);
    this->status = validate_request(serv);
    // std::cout << "Key: "  << "=== Value: "  << std::endl;
    // for (std::map<std::string, std::string>::iterator it = this->query_params.begin();it != this->query_params.end(); ++it) 
    // std::cout << it->first <<"  " << it->second << std::endl;
}
