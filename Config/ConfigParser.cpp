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
    std::cout << "Locations for server " << serverConfig.server_name << "( " << serverConfig.host << " )\n";
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

void ConfigParser::copyServerConfig(const ServerConfig& source, ServerConfig& destination) {
    destination.host = source.host;
    destination.port = source.port;
    destination.server_name = source.server_name;
    destination.error_pages = source.error_pages; 
    destination.client_max_body_size = source.client_max_body_size;

    destination.locations.clear();

    for (std::map<std::string, LocationConfig>::const_iterator it = source.locations.begin(); it != source.locations.end(); ++it) {
        const std::string& path = it->first;
        const LocationConfig& locConfig = it->second;

        LocationConfig newLocConfig;
        newLocConfig.root = locConfig.root;
        newLocConfig.index = locConfig.index;
        newLocConfig.autoindex = locConfig.autoindex;
        newLocConfig.methods = locConfig.methods; // shallow copy
        newLocConfig.allow_upload = locConfig.allow_upload;
        newLocConfig.upload_store = locConfig.upload_store;
        newLocConfig.cgi_pass = locConfig.cgi_pass;
        newLocConfig.redirect = locConfig.redirect;
        destination.locations[path] = newLocConfig;
    }

    printLocationConfig(destination);
}

std::vector<ServerConfig> ConfigParser::parseServerConfigFile(const std::string& filename)
{
	std::ifstream configFile(filename.c_str());
	if (!configFile.is_open())
	{
			std::string msg("Could not open configuration file !");
			msg += filename;
            ConfigError error(msg);
            LOG(error.what());
			return std::vector<ServerConfig>();
    }

	std::string line;
	ServerConfig currentServer;
	LocationConfig currentLocation;
	bool inServerBlock = false;
	bool inLocationBlock = false;
	std::string currentLocationPath;

	std::vector<ServerConfig> servers;

	while (std::getline(configFile, line)) {
		std::istringstream iss(line);
		std::string token;
		iss >> token;

		if (token == "server") {
			inServerBlock = true;
			currentServer = ServerConfig();
			//std::cout << "[CONF]: Found New Server" << std::endl;

		} else if (token == "location") {
			inLocationBlock = true;
			iss >> currentLocationPath;
			currentLocation = LocationConfig();
			//std::cout << "[CONF]: Found New Endpoint" << std::endl;
		} else if (token == "}") {
			if (inLocationBlock) {
				inLocationBlock = false;
				currentServer.locations[currentLocationPath] = currentLocation;
			} else if (inServerBlock) {
				inServerBlock = false;
				servers.push_back(currentServer);
			}
		} else if (inServerBlock && (token.size() > 1)) {
			if (token == "host") {
				iss >> currentServer.host;
			} else if (token == "port") {
				iss >> currentServer.port;
			} else if (token == "server_name") {
				iss >> currentServer.server_name;
			} else if (token == "error_page") {
				int errorCode;
				std::string errorPage;
				iss >> errorCode >> errorPage;
				currentServer.error_pages[errorCode] = errorPage;
			} else if (token == "client_max_body_size" && !inLocationBlock) {
				std::string sizeStr;
				iss >> sizeStr;
				if (!(sizeStr.empty()))
					currentServer.client_max_body_size = parseSize(sizeStr);
				else
					currentServer.client_max_body_size = parseSize("20K");
			} else if (inLocationBlock) {
				if (token == "root") {
					iss >> currentLocation.root;
					//std::cout << "[CONF]: Set Root " << currentLocation.root << std::endl;
				} else if (token == "index") {
					iss >> currentLocation.index;
					//std::cout << "[CONF]: Set Index " << currentLocation.index << std::endl;

				} else if (token == "autoindex") {
					std::string value;
					iss >> value;
					currentLocation.autoindex = (value == "on");
					//std::cout << "[CONF]: Set autoIndex " << currentLocation.autoindex << std::endl;
				} else if (token == "method") {
					std::string method;
					while (iss >> method) {
						currentLocation.methods.push_back(method);
						//std::cout << "[CONF]: Added allowed method " << method << std::endl;
					}
				} else if (token == "allow_upload") {
					std::string value;
					iss >> value;
					currentLocation.allow_upload = (value == "on");
					//std::cout << "[CONF]: Allow Uppload " << currentLocation.allow_upload << std::endl;
				} else if (token == "upload_store") {
					iss >> currentLocation.upload_store;
					//std::cout << "[CONF]: upload_store " << currentLocation.upload_store << std::endl;
				} else if (token == "cgi_pass") {
					iss >> currentLocation.cgi_pass;
					//std::cout << "[CONF]: cgi-pass " << currentLocation.cgi_pass << std::endl;
				} else if (token == "client_max_body_size")	{
					std::string sizeStr;
					iss >> sizeStr;
					if (!(sizeStr.empty()))
						currentLocation.client_max_body_size = parseSize(sizeStr);
					else
						currentLocation.client_max_body_size = parseSize("20K");
				} else if (token == "return") {
						int statusCode;
						std::string redirectUrl;

						iss >> statusCode;
						if (statusCode == 301) {
							iss >> redirectUrl;
							currentLocation.redirect = redirectUrl;
							//std::cout << "[CONF]: Set redirect " << currentLocation.redirect << std::endl;
						}
				}
			}
		}
	}
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
