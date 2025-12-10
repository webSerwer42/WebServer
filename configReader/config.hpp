//Copyright [2025] <Antoni Gorski> <agorski@student.42warsaw.pl>

#pragma once

#include <string>
#include <vector>
#include <map>

struct LocationConfig {
    std::string client_max_body_size;
    std::map<int, std::string> error_pages;
    std::vector<std::string> allow_methods;
    bool autoindex;
    std::string index;
    std::string upload_dir;
    std::string cgi_path;
    std::string cgi_ext;
    std::string root;
    bool has_redirect;           // Czy lokacja ma redirect
    int redirect_code;           // 301, 302, 303, 307, 308
    std::string redirect_url;    // Docelowy URL
    LocationConfig() : has_redirect(false), redirect_code(0) {}
};

struct ServerConfig {
    std::string listen_port;
    std::string server_name;
    std::string host;

    std::string client_max_body_size;
    std::map<int, std::string> error_pages;
    std::vector<std::string> allow_methods;
    bool autoindex;
    std::string index;
    std::string upload_dir;
    std::string cgi_path;
    std::string cgi_ext;
    std::string root;
    bool has_redirect;          // Czy lokacja ma redirect
    int redirect_code;           // 301, 302, 303, 307, 308
    std::string redirect_url;    // Docelowy URL
    std::map<std::string, LocationConfig> locations;
    ServerConfig() : has_redirect(false), redirect_code(0) {}
};

class Config {
private:
    std::vector<ServerConfig> servers;
    void parseLines(std::ifstream& file);
    
    public:
    Config(const std::string& filename);
    const std::vector<ServerConfig>& getServers() const;
    void printConfigs() const;
    static LocationConfig getMergedLocationConfig(
        const ServerConfig& server,
        const LocationConfig& location);
};
