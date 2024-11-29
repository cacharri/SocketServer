#include "ConfigParser.hpp"

ConfigParser::ConfigParser() {
}

ConfigParser::ConfigParser(const ConfigParser& copy)
{
	
}
ConfigParser& ConfigParser::operator=(const ConfigParser& assign)
{
	(void)assign;
	return (*this);
}

ConfigParser::~ConfigParser()
{
	
}

void ConfigParser::printLocationConfig(const ServerConfig& serverConfig) {
    std::cout << "Locations for server " << serverConfig.server_name << "( " << *(serverConfig.ports.begin()) << " )\n";
    std::map<std::string, LocationConfig>::const_iterator it;
    for (it = serverConfig.locations.begin(); it != serverConfig.locations.end(); ++it) {
        const std::string& path = it->first;
        const LocationConfig& locConfig = it->second;

        std::cout << "  Path: " << path << "\n";
        std::cout << "    Root: " << locConfig.root << "\n";
        std::cout << "    Index: " << locConfig.index << "\n";
		if (locConfig.autoindex)
			std::cout << "    Autoindex: on \n";
        std::cout << "    Allowed Methods: ";
        
        std::vector<std::string>::const_iterator methodIt;
        for (methodIt = locConfig.methods.begin(); methodIt != locConfig.methods.end(); ++methodIt) {
            std::cout << *methodIt << " ";
        }
        
        std::cout << "\n";
		if (locConfig.allow_upload)
			std::cout << "    allow_upload: on \n";
		if (!(locConfig.upload_store.empty()))
			std::cout << "    Upload Store: on \n";
		if (!(locConfig.cgi_pass.empty()))
			std::cout << "    cgi_pass: " << locConfig.cgi_pass << "\n";
    }
}



void ConfigParser::copyServerConfig(const ServerConfig& source, ServerConfig& destination)
{
    destination.interface = source.interface;
	if (!(source.ports.empty()))
	{
		for (std::vector<int>::const_iterator it = source.ports.begin(); it != source.ports.end(); it++)
			destination.ports.push_back((*it));
	}
    destination.server_name = source.server_name;
    destination.error_pages = source.error_pages; 
    destination.client_max_body_size = source.client_max_body_size;

    destination.locations.clear();

    for (std::map<std::string, LocationConfig>::const_iterator it = source.locations.begin(); it != source.locations.end(); it++) {
        const std::string& path = it->first;
        const LocationConfig& locConfig = it->second;

        LocationConfig newLocConfig;

        newLocConfig.root = it->second.root;
        std::cout << "new endpoint rooted to " << it->second.root << std::endl;
        newLocConfig.index = it->second.index;
        newLocConfig.autoindex = it->second.autoindex;
        newLocConfig.methods = it->second.methods;
        newLocConfig.allow_upload = it->second.allow_upload;
        newLocConfig.upload_store = it->second.upload_store;
        newLocConfig.cgi_pass = it->second.cgi_pass;
        newLocConfig.redirect = it->second.redirect;
        std::cout << "new endpoint redirect " << it->second.redirect << std::endl;
		newLocConfig.redirect_type = it->second.redirect_type;
        newLocConfig.client_max_body_size = it->second.client_max_body_size;
        //std::cout << "max body size en copy_server_config_es " << newLocConfig.client_max_body_size << std::endl;
        //std::cout << "New location" << path << " with SIZE " << newLocConfig.client_max_body_size << std::endl;
        destination.locations[path] = newLocConfig;
    }

    printLocationConfig(destination);
}

void                        ConfigParser::setDefaultErrorPages(ServerConfig& destination)
{
    destination.error_pages[103] = "var/www/error-pages/103.html";
	destination.error_pages[400] = "var/www/error-pages/400.html";
	destination.error_pages[403] = "var/www/error-pages/403.html";
	destination.error_pages[404] = "var/www/error-pages/404.html";
	destination.error_pages[405] = "var/www/error-pages/405.html";
	destination.error_pages[408] = "var/www/error-pages/408.html";
	destination.error_pages[413] = "var/www/error-pages/413.html";
	destination.error_pages[414] = "var/www/error-pages/414.html";
	destination.error_pages[500] = "var/www/error-pages/500.html";
	destination.error_pages[501] = "var/www/error-pages/501.html";
	destination.error_pages[502] = "var/www/error-pages/502.html";
	destination.error_pages[503] = "var/www/error-pages/503.html";
	destination.error_pages[504] = "var/www/error-pages/504.html";
}

bool ConfigParser::validateSyntax(const std::string& configFilePath) {
    std::ifstream file(configFilePath.c_str());
    if (!file.is_open()) {
        LOG("Configuration file cannot be opened: " + configFilePath);
        return false;
    }

    int openServerBlocks = 0, openLocationBlocks = 0;
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("server {") != std::string::npos) openServerBlocks++;
        if (line.find("location") != std::string::npos) openLocationBlocks++;
        if (line.find("}") != std::string::npos) {
            if (openLocationBlocks > 0) openLocationBlocks--;
            else if (openServerBlocks > 0) openServerBlocks--;
        }
    }
    file.close();
    if (openServerBlocks != 0 || openLocationBlocks != 0) {
        LOG("Unmatched blocks in configuration file.");
        return false;
    }
    return true;
}

bool isValidIP(const std::string& ip) {
    int dots = 0;
    size_t pos = 0;
    while ((pos = ip.find('.', pos)) != std::string::npos) {
        dots++;
        pos++;
    }
    return dots == 3;
}

bool    ConfigParser::validateServerParams(const ServerConfig& serverConfig) {
    if (serverConfig.interface.empty() || !isValidIP(serverConfig.interface))
    {
        std::string msg("Invalid or missing host in server configuration: ");
        msg += serverConfig.interface;
        LOG(msg);
        return false;
    }

    for (std::vector<int>::const_iterator it = serverConfig.ports.begin(); it != serverConfig.ports.end(); ++it) {
        if (*it <= 0 || *it > 65535) {
            std::string msg("Invalid port number: ");
            msg += intToString(*it); 
            LOG(msg); 
            return false;
        }
    }
    return true;
}

std::string ConfigParser::intToString(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

bool    ConfigParser::validateMethods(const std::vector<std::string>& methods) {
    static const char* validMethods[] = {"GET", "POST", "DELETE"};
    for (std::vector<std::string>::const_iterator it = methods.begin(); it != methods.end(); ++it) {
        for (size_t i = 0; i < sizeof(validMethods) / sizeof(validMethods[0]); i++) {
            if (*it == validMethods[i]) {
                return true;
            }
        }
    }
    std::string msg("Unsupported HTTP method");
    LOG(msg);
    return false;
}

std::vector<ServerConfig> ConfigParser::parseServerConfigFile(const std::string& filename)
{
    std::vector<ServerConfig> servers;
    if (!validateSyntax(filename)) {
        LOG("Invalid syntax in configuration file.");
        return servers;
    }

    std::ifstream configFile(filename.c_str());
    if (!configFile.is_open()) {
        std::string msg("Could not open configuration file: ");
        msg += filename;
        LOG(msg);
        return servers;
    }

    std::string line;
    ServerConfig currentServer;
    LocationConfig currentLocation;
    bool inServerBlock = false;
    bool inLocationBlock = false;
    std::string currentLocationPath;


    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "server") {
            if (inServerBlock) {
                LOG("Nested 'server' blocks are not allowed.");
                servers.clear();
                return servers;
            }
            inServerBlock = true;
            currentServer = ServerConfig();

        } else if (token == "location") {
            if (!inServerBlock) {
                LOG("'location' block found outside of a 'server' block.");
            }
            inLocationBlock = true;
            iss >> currentLocationPath;
            currentLocation = LocationConfig();
            //copia el max body size del server al cliente, para cubrir l caso de que no haya max size en upload
            currentLocation.client_max_body_size = currentServer.client_max_body_size;

        } else if (token == "}") {
            if (inLocationBlock) {
                inLocationBlock = false;

                if (validateMethods(currentLocation.methods) == false)
                {
                    servers.clear();
                    return servers;
                }
                currentServer.locations[currentLocationPath] = currentLocation;

            } else if (inServerBlock) {
                inServerBlock = false;

                // Validar parámetros del servidor antes de agregarlo a la lista
                if (validateServerParams(currentServer) == false)
                {
                    servers.clear();
                    return servers;
                }
                servers.push_back(currentServer);
            }

        } else if (inServerBlock && !inLocationBlock && token.size() > 1) {
            // Procesar configuraciones de servidores
            if (token == "host") {
                iss >> currentServer.interface;

            } else if (token == "port") {
                int in_port;
                iss >> in_port;
                currentServer.ports.push_back(in_port);

            } else if (token == "server_name") {
                iss >> currentServer.server_name;

            } else if (token == "error_page") {
                int errorCode;
                std::string errorPage;
                iss >> errorCode >> errorPage;
                currentServer.error_pages[errorCode] = errorPage;

            } else if (token == "client_max_body_size") {
                std::string sizeStr;
                iss >> sizeStr;
                std::cout <<"Server MAxSize: "<< sizeStr << std::endl;
                if (!sizeStr.empty()) {
                    currentServer.client_max_body_size = parseSize(sizeStr);
                }

            }
        }
        else if (inLocationBlock) {
                // Procesar configuraciones de ubicaciones
                if (token == "root") {
                    iss >> currentLocation.root;

                } else if (token == "index") {
                    iss >> currentLocation.index;

                } else if (token == "autoindex") {
                    std::string value;
                    iss >> value;
                    currentLocation.autoindex = (value == "on");

                } else if (token == "method") {
                    std::string method;
                    while (iss >> method) {
                        currentLocation.methods.push_back(method);
                    }

                } else if (token == "allow_upload") {
                    std::string value;
                    iss >> value;
                    currentLocation.allow_upload = (value == "on");

                } else if (token == "upload_store") {
                    iss >> currentLocation.upload_store;

                } else if (token == "cgi_pass") {
                    iss >> currentLocation.cgi_pass;

                } else if (token == "client_max_body_size") {
                    std::string sizeStr;
                    iss >> sizeStr;
                    std::cout << "Location Size:"<< sizeStr << std::endl;
                    if (!sizeStr.empty()) {
                        currentLocation.client_max_body_size = parseSize(sizeStr);
                    }

                } else if (token == "return") {
                    int statusCode;
                    std::string redirectUrl;
                    iss >> statusCode >> redirectUrl;
                    currentLocation.redirect = redirectUrl;
                    currentLocation.redirect_type = statusCode;
                }
        }
    }

    configFile.close();
    return servers;
}


size_t ConfigParser::parseSize(const std::string& sizeStr)
{
    size_t size = 0;
    char unit = sizeStr[sizeStr.size() - 1];

    if (unit == 'M') {
        size = atoi(sizeStr.substr(0, sizeStr.size() - 1).c_str()) * 1024 * 1024;
    } else if (unit == 'K') {
        size = atoi(sizeStr.substr(0, sizeStr.size() - 1).c_str()) * 1024;
    } else {
        size = atoi(sizeStr.c_str());
    }

	//std::cout << "Parsed size; " << size << std::endl;
    return size;
}

//Exceptions handling
ConfigParser::ConfigError::ConfigError(const std::string& error): error_string(error)
{
}

ConfigParser::ConfigError::~ConfigError() throw()
{
}

const char * ConfigParser::ConfigError::what() const throw()
{
	return error_string.c_str();
}
