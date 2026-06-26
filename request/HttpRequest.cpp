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

std::string parse_path(const std::string& str)
{
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

void check_path_get(validat& requ, const std::string& fs_path, const ServerConfig::LocationConfig* loc, const std::string& method)
{
    struct stat st;

    // First: does fs_path exist?
    if (stat(fs_path.c_str(), &st) != 0)
    {
        
        if (errno == ENOENT || errno == ENOTDIR)
        {
            requ.code = 404;
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
    // POST (upload-like behavior)
    // -------------------------
    if (method == "POST")
    {
        // For uploads you typically POST to a directory endpoint like /upload/
        // We accept POST only if it's a directory and writable.
        if (!S_ISDIR(st.st_mode)) {
            // You can choose 403 or 409 here. 403 is okay for your current style.
            requ.code = 403;
            requ.path = "";
            return;
        }

        // Must be able to write into this directory
        if (access(fs_path.c_str(), W_OK | X_OK) != 0) {
            requ.code = 403;
            requ.path = "";
            return;
        }

        requ.code = 200;      // OK to proceed with upload handling
        requ.path = fs_path;  // directory where upload handler should write
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
        std::cout<< "55" << fs_path <<std::endl;
        requ.code=403;
        requ.path="";
        return;
    }
    if (access(fs_path.c_str(), R_OK) != 0) {
        if (errno == EACCES || errno == EPERM) {
            std::cout<< "66" << fs_path <<std::endl;
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

}

validat HttpRequest::validate_request(const ServerConfig& serv)
{
    std::string current_path = this->path;
    const ServerConfig::LocationConfig* loc = best_match_location(current_path, serv);
    validat requ;

    if (loc && loc->redirect.enabled)
    {
        requ.code = loc->redirect.code;
        int redirects_followed = 0;
        int max_redirects = serv.locations.size();
        current_path = loc->redirect.target;

        while (loc && loc->redirect.enabled && redirects_followed < max_redirects)
        {
            current_path = loc->redirect.target;
            redirects_followed++;
            loc = best_match_location(current_path, serv);
        }

        if (redirects_followed >= max_redirects)
        {
            requ.code = 508;
            return requ;
        }
        
        std::string root = serv.root;
        if (loc && !loc->root.empty())
            root = loc->root;
        
        // Remove trailing / from root
        if (!root.empty() && root.back() == '/') {
            root = root.substr(0, root.length() - 1);
        }
        
        std::string path_to_use = current_path;
        
        // Only strip prefix if location has custom root and is not root location
        if (loc && !loc->root.empty() && loc->prefix != "/") {
            if (path_to_use.find(loc->prefix) == 0) {
                path_to_use = path_to_use.substr(loc->prefix.length() - 1);
            }
        }
        
        // Ensure leading /
        if (!path_to_use.empty() && path_to_use[0] != '/') {
            path_to_use = "/" + path_to_use;
        }
        
        std::string fs_path = root + path_to_use;

        struct stat st;
        if (stat(fs_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
        {
            if (loc && !loc->index.empty())
            {
                for (size_t i = 0; i < loc->index.size(); i++)
                {
                    std::string index_path = fs_path + "/" + loc->index[i];
                    if (stat(index_path.c_str(), &st) == 0 && S_ISREG(st.st_mode))
                    {
                        this->redirect_target = current_path + "/" + loc->index[i];
                        return requ;
                    }
                }
            }
        }
    }

    if (!method_allowed(this->method, loc)) 
    {
        requ.code = 405;
        requ.path = "";
        return requ;
    }
    
    if (loc && loc->upload.enabled && !loc->upload.dir.empty())
    {
        if (this->path.find(loc->prefix) == 0)
        {
            struct stat st;
            if (stat(loc->upload.dir.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
            {
                // Remove trailing / from upload dir
                std::string upload_dir = loc->upload.dir;
                if (!upload_dir.empty() && upload_dir.back() == '/') {
                    upload_dir = upload_dir.substr(0, upload_dir.length() - 1);
                }
                
                std::string path_to_use = this->path;
                
                // Only strip prefix if location has custom root and is not root location
                if (loc && !loc->root.empty() && loc->prefix != "/") {
                    if (path_to_use.find(loc->prefix) == 0) {
                        path_to_use = path_to_use.substr(loc->prefix.length() - 1);
                    }
                }
                
                // Ensure leading /
                if (!path_to_use.empty() && path_to_use[0] != '/') {
                    path_to_use = "/" + path_to_use;
                }
                
                requ.path = upload_dir + path_to_use;
                requ.code = 200;
                return requ;
            }
            else
            {
                requ.code = 500;
                requ.path = "";
                return requ;
            }
        }
    }
    
    std::string root = serv.root;
    if (loc && !loc->root.empty())
        root = loc->root;
    
    // Remove trailing / from root
    if (!root.empty() && root.back() == '/') {
        root = root.substr(0, root.length() - 1);
    }
    
    std::string path_to_use = this->path;
    
    // Only strip prefix if location has custom root and is not root location
    if (loc && !loc->root.empty() && loc->prefix != "/") {
        if (path_to_use.find(loc->prefix) == 0) {
            path_to_use = path_to_use.substr(loc->prefix.length() - 1);
        }
    }
    
    // Ensure leading /
    if (!path_to_use.empty() && path_to_use[0] != '/') {
        path_to_use = "/" + path_to_use;
    }
    
    std::string fs_path = root + path_to_use;

    check_path_get(requ, fs_path, loc, this->method);
    return requ;
}

HttpRequest::HttpRequest(const std::string& raw_request, const ServerConfig& serv)
{
    const size_t MAX_BODY = serv.client_max_body_size;
    const size_t MAX_PATH = 2048;

    try {
        if (raw_request.empty())
            throw std::invalid_argument("Request is empty");

        std::vector<std::string> lines;
        std::stringstream ss(raw_request);
        std::string line;
        this->confurm_path = "";
        while (std::getline(ss, line)) 
            lines.push_back(line);

        if (lines.empty())
            throw std::invalid_argument("Request is not valid: no lines present");

        if (!valid_request_line(lines[0]))
            throw std::invalid_argument("Malformed request line");

        this->method = parse_method(lines[0]);
        this->path = parse_path(lines[0]);
        this->version = parse_version(lines[0]);

        if (this->path.size() > MAX_PATH) {
            this->status = 414;
            this->confurm_path = "";
            return;
        }

        this->headers = pars_heders(lines);

        if ((this->version == "HTTP/1.1" || this->version == "HTTP/2.0") &&
            this->headers.find("Host") == this->headers.end()) {
            this->status = 400;
            this->confurm_path = "";
            return;
        }

        if (this->method == "POST"  &&
            this->headers.find("Content-Length") == this->headers.end()) {
            this->status = 411;
            this->confurm_path = "";
            return;
        }

        this->body = pars_body(lines);

        if (this->method == "POST" ) {
            std::map<std::string, std::string>::const_iterator it = this->headers.find("Content-Length");
            if (it != this->headers.end()) {
                std::istringstream iss(it->second);
                size_t body_len = 0;
                iss >> body_len;

                if (!iss.fail() && body_len > MAX_BODY) {
                    this->status = 413;
                    return;
                }
            }
        }

        if (this->method == "POST") {
            if (this->body.size() > MAX_BODY) {
                this->status = 413;
                return;
            }
        }

        this->query_params = pars_query(lines[0]);
        validat validation_result = validate_request(serv);  // ✅ Call once
        this->status = validation_result.code;
        this->confurm_path = validation_result.path;

        // ========== NEW: CGI CHECK ==========
        this->is_cgi = false;  // Initialize to false

        if (this->status == 200 || this->status == 1001)
        {
            // Extract query string from path (everything after "?")
            size_t query_pos = this->path.find('?');
            if (query_pos != std::string::npos)
            {
                this->query_string = this->path.substr(query_pos + 1);
            } else {
                this->query_string = "";
            }

            // Check if this is a CGI request
            this->detect_cgi_request(serv);

            // If it's CGI and status is still 200, setup environment
            if (this->is_cgi && (this->status == 200 || this->status == 1001)) {
                this->setup_cgi_environment(serv);
            }
        }
        // ====================================
    }
    catch (const std::logic_error& e)
    {
        if (std::string(e.what()) == "501")
            this->status = 501;
        else
            this->status = 400;
        this->confurm_path = "";
        this->is_cgi = false;  // NEW
    }
    catch (const std::exception& e) {
        this->status = 400;
        this->confurm_path = "";
        this->is_cgi = false;  // NEW
    }
}

void HttpRequest::reqq()
{
    std::cout
        << "method: " << this->method << "\n"
        << "path: " << this->path << "\n"
        << "confurm_path: " << this->confurm_path << "\n"
        << "version: " << this->version << "\n"
        << "body: " << this->body << "\n"
        << "status: " << this->status << "\n"
        << "redirect_target: " << this->redirect_target << "\n";

    for (std::map<std::string, std::string>::iterator it = this->headers.begin(); 
         it != this->headers.end(); ++it)
        std::cerr << "header: " << it->first << " = " << it->second << "\n";

    for (std::map<std::string, std::string>::iterator it = this->query_params.begin(); 
         it != this->query_params.end(); ++it)
        std::cerr << "query: " << it->first << " = " << it->second << "\n";

    // ========== NEW: CGI INFO ==========
    std::cout << "\n--- CGI Information ---\n";
    std::cout << "is_cgi: " << (this->is_cgi ? "true" : "false") << "\n";
    
    if (this->is_cgi) {
        std::cout << "cgi_script_path: " << this->cgi_script_path << "\n"
                  << "cgi_extension: " << this->cgi_extension << "\n"
                  << "cgi_interpreter: " << this->cgi_interpreter << "\n"
                  << "query_string: " << this->query_string << "\n";
        
        std::cout << "\nCGI Environment Variables:\n";
        for (std::map<std::string, std::string>::iterator it = this->cgi_env.begin(); 
             it != this->cgi_env.end(); ++it)
            std::cerr << "  " << it->first << " = " << it->second << "\n";
    }
    // ====================================
}

bool HttpRequest::detect_cgi_request(const ServerConfig& serv)
{
    const ServerConfig::LocationConfig* best = best_match_location(this->path, serv);
    
    if (!best || best->cgi.empty())
    {
        is_cgi = false;
        return false;
    }

    size_t dot_pos = confurm_path.find_last_of('.');
    if (dot_pos == std::string::npos)
    {
        is_cgi = false;
        return false;
    }

    cgi_extension = confurm_path.substr(dot_pos);

    std::map<std::string, std::string>::const_iterator cgi_it = best->cgi.find(cgi_extension);
    if (cgi_it == best->cgi.end())
    {
        status = 403;
        is_cgi = false;
        return false;
    }

    cgi_interpreter = cgi_it->second;
    cgi_script_path = confurm_path;
    is_cgi = true;
    return true;
}


void HttpRequest::setup_cgi_environment(const ServerConfig& serv)
{
    if (!is_cgi) return;
    cgi_env.clear();
    // ===== REQUEST METHOD & DATA =====
    cgi_env["REQUEST_METHOD"] = method;
    cgi_env["QUERY_STRING"] = query_string;
    cgi_env["CONTENT_LENGTH"] = intToString(body.length());
    
    // ===== CONTENT TYPE =====
    std::map<std::string, std::string>::const_iterator it_content = headers.find("Content-Type");
    if (it_content != headers.end()) {
        cgi_env["CONTENT_TYPE"] = it_content->second;
    } else {
        cgi_env["CONTENT_TYPE"] = "";
    }
    
    // ===== FILE PATH INFORMATION =====
    cgi_env["SCRIPT_NAME"] = path;
    cgi_env["SCRIPT_FILENAME"] = cgi_script_path;
    cgi_env["PATH_INFO"] = path;
    
    // ===== SERVER INFORMATION =====
    cgi_env["SERVER_NAME"] = serv.server_name;
    cgi_env["SERVER_PORT"] = intToString(serv.listens[0].port);
    cgi_env["SERVER_PROTOCOL"] = "HTTP/1.1";
    cgi_env["GATEWAY_INTERFACE"] = "CGI/1.1";
    
    // ===== HTTP HEADERS TO ENVIRONMENT =====
    std::map<std::string, std::string>::const_iterator header_it;
    for (header_it = headers.begin(); header_it != headers.end(); ++header_it) {
        std::string key = header_it->first;
        
        // Skip Content-Type and Content-Length
        if (key == "Content-Type" || key == "Content-Length") {
            continue;
        }
        
        // Convert to uppercase and replace '-' with '_'
        std::transform(key.begin(), key.end(), key.begin(), ::toupper);
        for (std::string::iterator c_it = key.begin(); c_it != key.end(); ++c_it) {
            if (*c_it == '-') *c_it = '_';
        }
        
        // Add "HTTP_" prefix
        key = "HTTP_" + key;
        
        cgi_env[key] = header_it->second;
    }
}