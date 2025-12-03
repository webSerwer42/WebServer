//Copyright [2025] <Antoni Gorski> <agorski@student.42warsaw.pl>

#include "config.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

void trim(std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if(start == std::string::npos) {
        s.clear();
        return;
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    s = s.substr(start, end - start + 1);
    if (!s.empty() && s[s.size() - 1] == ';')
        s.erase(s.size() - 1);
}

void Config::parseLines(std::ifstream& file) {
    std::string line;
    ServerConfig currentServer;
    LocationConfig currentLocation;
    std::string currentLocationPath;
    bool inServer = false;
    bool inLocation = false;

    while (std::getline(file, line)) {
        trim(line);
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "server") {
            if (inServer) {
                if (inLocation)
                    currentServer.locations[currentLocationPath] = currentLocation;
                servers.push_back(currentServer);
            }
            // Inicjalizuj nowy serwer (domyślnie: autoindex=false, pozostałe pola puste)
            currentServer = ServerConfig();
            // Lokacja dziedziczy z serwera (autoindex pusty = weź z serwera)
            currentLocation = LocationConfig();
            currentLocationPath.clear();
            inServer = true;
            inLocation = false;
        }
        else if (key == "listen") {
            std::string p; iss >> p; 
            currentServer.listen_port = p;
        }
        else if (key == "server_name") {
            std::string sn; iss >> sn; 
            currentServer.server_name = sn;
        }
        else if (key == "host") {
            std::string h; iss >> h; 
            currentServer.host = h;
        }
        else if (key == "client_max_body_size") {
            std::string val; iss >> val;
            if (inLocation)
                currentLocation.client_max_body_size = val;
            else
                currentServer.client_max_body_size = val;
        }
        else if (key == "error_page") {
            int code; std::string val; 
            iss >> code >> val;
            if (inLocation)
                currentLocation.error_pages[code] = val;
            else
                currentServer.error_pages[code] = val;
        }
        else if (key == "methods") {
            std::vector<std::string> methods;
            std::string m;
            while (iss >> m)
                methods.push_back(m);
            if (inLocation)
                currentLocation.allow_methods = methods;
            else
                currentServer.allow_methods = methods;
        }
        else if (key == "directory_listing") {
            std::string val; iss >> val;
            bool autoindex_val = (val == "on");
            if (inLocation) {
                currentLocation.autoindex = autoindex_val;
            } else {
                currentServer.autoindex = autoindex_val;
            }
        }
        else if (key == "index") {
            std::string idx; iss >> idx;
            if (inLocation)
                currentLocation.index = idx;
            else
                currentServer.index = idx;
        }
        else if (key == "upload") {
            std::string upload_dir; iss >> upload_dir;
            if (inLocation)
                currentLocation.upload_dir = upload_dir;
            else
                currentServer.upload_dir = upload_dir;
        }
        else if (key == "cgi_path") {
            std::string cgi_path; iss >> cgi_path;
            if (inLocation)
                currentLocation.cgi_path = cgi_path;
            else
                currentServer.cgi_path = cgi_path;
        }
        else if (key == "cgi_ext") {
            std::string cgi_ext; iss >> cgi_ext;
            if (inLocation)
                currentLocation.cgi_ext = cgi_ext;
            else
                currentServer.cgi_ext = cgi_ext;
        }
        else if (key == "root") {
            std::string r; iss >> r;
            if (inLocation)
                currentLocation.root = r;
            else
                currentServer.root = r;
        }
        else if (key == "route") {
            if (inLocation)
                currentServer.locations[currentLocationPath] = currentLocation;
            iss >> currentLocationPath;
            currentLocation = LocationConfig();
            inLocation = true;
        }
    }

    if (inServer) {
        if (inLocation)
            currentServer.locations[currentLocationPath] = currentLocation;
        servers.push_back(currentServer);
    }
}

Config::Config(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file.is_open())
        throw std::runtime_error("Cannot open config file");
    parseLines(file);
}

const std::vector<ServerConfig>& Config::getServers() const {
    return servers;
}

LocationConfig Config::getMergedLocationConfig(
    const ServerConfig& server, 
    const LocationConfig& location) 
{
    LocationConfig merged = location;
    
    // Jeśli location nie ma parametru, weź z serwera
    if (merged.client_max_body_size.empty()) 
        merged.client_max_body_size = server.client_max_body_size;
    
    if (merged.allow_methods.empty()) 
        merged.allow_methods = server.allow_methods;
    
    if (merged.index.empty()) 
        merged.index = server.index;
    
    if (merged.root.empty()) 
        merged.root = server.root;
    
    if (merged.upload_dir.empty()) 
        merged.upload_dir = server.upload_dir;
    
    if (merged.cgi_path.empty()) 
        merged.cgi_path = server.cgi_path;
    
    if (merged.cgi_ext.empty()) 
        merged.cgi_ext = server.cgi_ext;
    
    // Dla error_pages - merge map (location nadpisuje serwer)
    if (merged.error_pages.empty()) 
        merged.error_pages = server.error_pages;
    else {
        // Dodaj błędy z serwera, których nie ma w location
        for (std::map<int, std::string>::const_iterator it = server.error_pages.begin();
             it != server.error_pages.end(); ++it) {
            if (merged.error_pages.find(it->first) == merged.error_pages.end()) {
                merged.error_pages[it->first] = it->second;
            }
        }
    }
    return merged;
}