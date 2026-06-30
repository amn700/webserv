#include "HttpRequest.hpp"


bool valid_request_line(const std::string& line) 
{
    size_t firstSpace = line.find(' ');
    if (firstSpace == std::string::npos)
        return false;
    size_t secondSpace = line.find(' ', firstSpace + 1);
    if (secondSpace == std::string::npos)
        return false;
    if (line.find(' ', secondSpace + 1) != std::string::npos)
        return false;
    std::string method = line.substr(0, firstSpace);
    // Note: method validation is done in parse_method, but this can also catch "OPTIONS ..." as invalid
    if (!(method == "GET" || method == "POST" || method == "DELETE"))
        throw std::logic_error("501");
    std::string version = line.substr(secondSpace + 1);
    if (version.substr(0,5) != "HTTP/")
        return false;
    return true;
}

std::string parse_method(const std::string& str)
{
    size_t spacePos = str.find(' ');
    std::string firstWord = str.substr(0, spacePos);
    if (firstWord == "GET" || firstWord == "POST" || firstWord == "DELETE")
        return firstWord;
    throw std::logic_error("501"); //this must not be throw  // throw a logic_error to distinguish from 400
}

std::string parse_version(const std::string& str) 
{
    size_t lastSpace = str.find_last_of(' ');
    std::string lastWord = str.substr(lastSpace + 1);
    return lastWord;
}

std::string pars_body(const std::vector<std::string>& lines) {
    unsigned int i;
    for (i = 1; i < lines.size(); i++) {
        if (lines[i].find_first_not_of(" \t\n\r\f\v") == std::string::npos)
            break;
    }
    i += 1;
    if (i == lines.size())
        return "";
    std::string ret;
    while (i < lines.size()) {
        ret += lines[i];
        if (i != lines.size() - 1)
            ret += "\n";
        i++;
    }
    return ret;
}

std::string parse_path(const std::string& str) {
    size_t firstSpace = str.find(' ');
    size_t secondSpace = str.find(' ', firstSpace + 1);
    std::string secondWord = str.substr(firstSpace + 1, secondSpace - firstSpace - 1);
    if (secondWord.find('?') != std::string::npos) {
        size_t firstSepert = secondWord.find('?');
        return secondWord.substr(0, firstSepert);
    } else {
        return secondWord;
    }
}

std::string trim_str(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t");
    if (start == std::string::npos)
        return "";
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

std::map<std::string, std::string> pars_heders(const std::vector<std::string>& lines)
{
    std::map<std::string, std::string> ret;
    for (unsigned int i = 1; i < lines.size(); i++) {
        if (lines[i].find_first_not_of(" \t\n\r\f\v") == std::string::npos)
            break;
        size_t colon_pos = lines[i].find(':');
        if (colon_pos == std::string::npos || colon_pos == 0)
            throw std::invalid_argument("Malformed header line");
        std::string key = trim_str(lines[i].substr(0, colon_pos));
        std::string value = trim_str(lines[i].substr(colon_pos + 1));
        if (key.empty())
            throw std::invalid_argument("Empty header key");
        ret.insert(std::make_pair(key, value));
    }
    return ret;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter))
        result.push_back(token);
    return result;
}

std::map<std::string, std::string> pars_query(const std::string& str) {
    std::map<std::string, std::string> ret;
    size_t qmark = str.find('?');
    if(qmark == std::string::npos)
        return ret;
    size_t spacePos = str.find(' ', qmark);
    std::string theword = (spacePos == std::string::npos)
        ? str.substr(qmark + 1)
        : str.substr(qmark + 1, spacePos - qmark - 1);
    std::vector<std::string> result = split(theword , '&');
    for(unsigned int i = 0; i < result.size();i++) {
        size_t colon_pos = result[i].find('=');
        std::string key;
        std::string value;
        if(colon_pos == std::string::npos) {
            key = result[i];
            value = "";
        } else {
            key = result[i].substr(0, colon_pos);
            value = result[i].substr(colon_pos + 1);
        }
        ret.insert(std::make_pair(key, value));
    }
    return ret;
}

const ServerConfig::LocationConfig* best_match_location(const std::string& path, const ServerConfig& serv)
{
    const ServerConfig::LocationConfig* best = NULL;
    size_t bestLen = 0;
    for (size_t i = 0; i < serv.locations.size(); ++i) {
        const std::string& prefix = serv.locations[i].prefix;
        if (path.rfind(prefix, 0) == 0) {
            if (prefix.size() > bestLen) {
                bestLen = prefix.size();
                best = &serv.locations[i];
            }
        }
    }
    return best;
}

std::string toLower(const std::string& str)
{
// #include "response/ResponseHandler.hp
    std::string result = str;
    for (size_t i = 0; i < result.size(); ++i)
        result[i] = std::tolower(result[i]);
    return result;
}

bool method_allowed(const std::string& method, const ServerConfig::LocationConfig* loc)
{
    if (!loc) return true;
    if (loc->methods.empty()) return true;
    return loc->methods.count(toLower(method)) != 0;
}

// void check_path_get(validat& requ, const std::string& fs_path, const ServerConfig::LocationConfig* loc)
// {
//     struct stat st;
//     std::string check_path = fs_path;

//     // If path is "/" (or equivalent), search for index files in the root
//     // if (fs_path == "/" || fs_path.empty())
//     // {
        
//     //     if (!loc || loc->index.empty())
//     //     {
//     //         requ.code = 403;
//     //         requ.path = "";
//     //         return;
//     //     }
//     //     for (size_t i = 0; i < loc->index.size(); ++i)
//     //     {
//     //         std::string index_path = loc->root + loc->index[i]; // Ensure loc->root ends without slash
//     //         struct stat index_st;
//     //         if (stat(index_path.c_str(), &index_st) == 0 && S_ISREG(index_st.st_mode) && access(index_path.c_str(), R_OK) == 0)
//     //         {
                
//     //             requ.code = 200;
//     //             requ.path = index_path;
//     //             return;
//     //         }
//     //     }
//     //     // No index file found, check autoindex
//     //     if (loc->autoindex) 
//     //     {
//     //         requ.code = 1001; // autoindex listing
//     //         requ.path = loc->root; // The directory to list
//     //         return;
//     //     }
//     //     else 
//     //     {
//     //         requ.code = 403;
//     //         requ.path = "";
//     //         return;
//     //     }
//     // }

//     // Original code (unchanged) follows for all other paths
//     if (stat(fs_path.c_str(), &st) != 0) 
//     {
//         if (errno == ENOENT || errno == ENOTDIR) {
//             requ.code=404;
//             requ.path="";
//             return;
//         }
//         if (errno == EACCES || errno == EPERM) {
//             requ.code=403;
//             requ.path="";
//             return;
//         }
//         requ.code=500;
//         requ.path="";
//         return;
//     }
//     if (S_ISDIR(st.st_mode)) 
//     {
//         // Check for index file in this directory
//         for (size_t i = 0; loc && i < loc->index.size(); i++) {
//             std::string index_path = fs_path + loc->index[i];
//             struct stat index_st;
//             if (stat(index_path.c_str(), &index_st) == 0 && S_ISREG(index_st.st_mode) && access(index_path.c_str(), R_OK) == 0)
//             {
//             //    std::cout<<"this---"<<fs_path  <<std::endl;
//             //    std::cout<<"that---"<< loc->index[i]<<std::endl;
//                 requ.code = 200;
//                 requ.path = index_path;
//                 return;
//             }
//         }
//         if (loc && loc->autoindex) {
//             requ.code = 1001;
//             requ.path = fs_path;
//             return;
//         }
//         requ.code=403;
//         requ.path="";
//         return;
//     }
//     if (access(fs_path.c_str(), R_OK) != 0) {
//         if (errno == EACCES || errno == EPERM) {
//             requ.code=403;
//             requ.path="";
//             return;
//         }
//         requ.code=500;
//         requ.path="";
//         return;
//     }
//     requ.code=200;
//     requ.path=fs_path;
//     return;
// }

// Replace check_path_get with a generic checker (keep the old name if you want)
void check_path_get(validat& requ,
                       const std::string& fs_path,
                       const ServerConfig::LocationConfig* loc,
                       const std::string& method)
{
    // -------------------------
    // POST (upload-like behavior)
    // -------------------------
    if (method == "POST")
    {
        if (!loc || !loc->upload.enabled)
        {
            requ.code = 403;
            requ.path = "";
            return;
        }

        if (loc->upload.dir.empty())
        {
            requ.code = 500;
            requ.path = "";
            return;
        }

        struct stat upload_st;
        if (stat(loc->upload.dir.c_str(), &upload_st) != 0)
        {
            if (errno == ENOENT || errno == ENOTDIR)
                requ.code = 404;
            else if (errno == EACCES || errno == EPERM)
                requ.code = 403;
            else
                requ.code = 500;
            requ.path = "";
            return;
        }

        if (!S_ISDIR(upload_st.st_mode)) {
            requ.code = 403;
            requ.path = "";
            return;
        }

        if (access(loc->upload.dir.c_str(), W_OK | X_OK) != 0) {
            if (errno == EACCES || errno == EPERM)
                requ.code = 403;
            else
                requ.code = 500;
            requ.path = "";
            return;
        }

        requ.code = 200;      // OK to proceed with upload handling
        requ.path = loc->upload.dir;  // directory where upload handler should write
        return;
    }

    struct stat st;

    // First: does fs_path exist?
    if (stat(fs_path.c_str(), &st) != 0)
    {
        if (errno == ENOENT || errno == ENOTDIR) {
            requ.code = 404;
            requ.path = "";
            return;
        }
        if (errno == ENAMETOOLONG) {
            requ.code = 414;
            requ.path = "";
            return;
        }
        if (errno == EACCES || errno == EPERM) {
            requ.code = 403;
            requ.path = "";
            return;
        }
        requ.code = 500;
        requ.path = "";
        return;
    }

    // -------------------------
    // DELETE
    // -------------------------
    if (method == "DELETE")
    {
        // For simplicity:
        // - forbid deleting directories (unless you want to support it)
        if (S_ISDIR(st.st_mode)) {
            requ.code = 403;
            requ.path = "";
            return;
        }

        // Check we can modify/remove. (Real POSIX delete checks parent dir W/X,
        // but this is a decent basic check for a school project.)
        if (access(fs_path.c_str(), W_OK) != 0) {
            requ.code = 403;
            requ.path = "";
            return;
        }

        requ.code = 200;     // deletable
        requ.path = fs_path; // file to delete
        return;
    }

    // -------------------------
    // GET (your existing behavior)
    // -------------------------


    // Original code (unchanged) follows for all other paths
    if (stat(fs_path.c_str(), &st) != 0) 
    {
        if (errno == ENOENT || errno == ENOTDIR) {
            requ.code=404;
            requ.path="";
            return;
        }
        if (errno == EACCES || errno == EPERM) {
            requ.code=403;
            requ.path="";
            return;
        }
        requ.code=500;
        requ.path="";
        return;
    }
    if (S_ISDIR(st.st_mode)) 
    {
        // Check for index file in this directory
        for (size_t i = 0; loc && i < loc->index.size(); i++)
        {
            std::string index_path = fs_path + loc->index[i];
            struct stat index_st;
            if (stat(index_path.c_str(), &index_st) == 0 && S_ISREG(index_st.st_mode) && access(index_path.c_str(), R_OK) == 0)
            {
                requ.code = 200;
                requ.path = index_path;
                return;
            }
        }
        if (loc && loc->autoindex) {
            requ.code = 1001;
            requ.path = fs_path;
            return;
        }
        requ.code=403;
        requ.path="";
        return;
    }
    if (access(fs_path.c_str(), R_OK) != 0) {
        if (errno == EACCES || errno == EPERM) {
            requ.code=403;
            requ.path="";
            return;
        }
        requ.code=500;
        requ.path="";
        return;
    }
    requ.code=200;
    requ.path=fs_path;
    return;
    

    // Regular file GET: must be readable
    // if (access(fs_path.c_str(), R_OK) != 0) {
    //     if (errno == EACCES || errno == EPERM) {
    //         requ.code = 403;
    //         requ.path = "";
    //         return;
    //     }
    //     requ.code = 500;
    //     requ.path = "";
    //     return;
    // }

    // requ.code = 200;
    // requ.path = fs_path;
}

validat HttpRequest::validate_request(const ServerConfig& serv)
{
    const ServerConfig::LocationConfig* loc = best_match_location(this->path, serv);
    validat requ;
    if (loc && loc->redirect.enabled) {
        this->redirect_target = loc->redirect.target;
        requ.path = "";
        requ.code = loc->redirect.code;
        return requ;
    }
    if (!method_allowed(this->method, loc)) 
    {
        requ.code = 405;
        requ.path = "";
        return requ;
    }
   
// (otherwise proceed to GET logic)
    std::string root = serv.root;
    if (loc && !loc->root.empty())
        root = loc->root;
    std::string fs_path = root + this->path; 
    check_path_get(requ, fs_path, loc ,this->method);
    return requ;
}

// --- The fixed, robust HttpRequest constructor. All error codes are correct. ---

HttpRequest::HttpRequest(const std::string& raw_request, const ServerConfig& serv)
{
    const size_t DEFAULT_MAX_BODY = 1024 * 1024 * 5;
    const size_t MAX_BODY = serv.client_max_body_size ? serv.client_max_body_size : DEFAULT_MAX_BODY;
    const size_t MAX_PATH = 2048; // 2048 bytes max URI/path size

    try {
        if (raw_request.empty())
            throw std::invalid_argument("Request is empty");

        std::vector<std::string> lines;
        std::stringstream ss(raw_request);
        std::string line;
        this->redirect_target = "";
        while (std::getline(ss, line)) 
            lines.push_back(line);

        if (lines.empty())
            throw std::invalid_argument("Request is not valid: no lines present");

        if (!valid_request_line(lines[0]))
            throw std::invalid_argument("Malformed request line");

        this->method = parse_method(lines[0]);
        this->path = parse_path(lines[0]);
        this->version = parse_version(lines[0]);

        // [COMPLETE] 414 URI Too Long
        if (this->path.size() > MAX_PATH) {
            this->status = 414;
            this->redirect_target = "";
            return;
        }

        this->headers = pars_heders(lines);

        // [COMPLETE] 400 Bad Request if Host header missing for HTTP/1.1+
        if ((this->version == "HTTP/1.1" || this->version == "HTTP/2.0") &&
            this->headers.find("Host") == this->headers.end()) {
            this->status = 400; // Bad Request
            this->redirect_target = "";
            return;
        }

        // [COMPLETE] 411 Length Required for POST/PUT sheck if Content-Length existed or not
        if ((this->method == "POST" || this->method == "PUT") &&
            this->headers.find("Content-Length") == this->headers.end()) {
            this->status = 411;
            this->redirect_target = "";
            return;
        }

        this->body = pars_body(lines);

        // [COMPLETE] 413 Payload Too Large
        if (this->method == "POST" || this->method == "PUT") 
{
        std::map<std::string, std::string>::const_iterator it = this->headers.find("Content-Length");
        if (it != this->headers.end()) 
    {
        std::istringstream iss(it->second);
        size_t body_len = 0;
        iss >> body_len;

        if (!iss.fail() && body_len > MAX_BODY) 
        {
            this->status = 413;
            return;
        }
    }
    // else: header is required! Check that separately for 411
}

// After reading the real body (defensive, "paranoia" check)
if (this->method == "POST" || this->method == "PUT") {
    if (this->body.size() > MAX_BODY) {
        this->status = 413;
        return;
    }
}

        this->query_params = pars_query(lines[0]);
        this->status = validate_request(serv).code;
        if (this->status == 200 || this->status == 1001)
            this->redirect_target = validate_request(serv).path;
    } 
    catch (const std::logic_error& e) 
    { // in constructor catch list
        if (std::string(e.what()) == "501")
            this->status = 501;
        else
            this->status = 400;
        this->redirect_target = "";
    }
    catch (const std::exception& e) 
    {
        this->status = 400;
        this->redirect_target = "";
    }
}


void HttpRequest::reqq()
{

 std::cout
  << "method: " << this->method << "\n"
  << "path: " << this->path << "\n"
  << "version: " << this->version << "\n"
  << "body: " << this->body << "\n"
  << "status: " << this->status << "\n"
  << "redirect_target: " << this->redirect_target << "\n";
for (std::map<std::string, std::string>::iterator it = this->headers.begin(); it != this->headers.end(); ++it)
  std::cerr << "header: " << it->first << " = " << it->second << "\n";
for (std::map<std::string, std::string>::iterator it = this->query_params.begin(); it != this->query_params.end(); ++it)
  std::cerr << "query: " << it->first << " = " << it->second << "\n";
}