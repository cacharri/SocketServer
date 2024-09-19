#include "ServerConfig.hpp"

ConfigParser::ConfigParser() {
}

ConfigParser::~ConfigParser() {
}

ConfigParser::ConfigParser(const ConfigParser& other) {

}

ConfigParser& ConfigParser::operator=(const ConfigParser& other) {
	if (this != &other)
	{

	}
	return *this;
}

std::vector<ServerConfig> ConfigParser::parseConfigFile(const std::string& filename) const {
	std::ifstream configFile(filename.c_str());
	if (!configFile.is_open())
	{
			std::string msg("Could not open configuration file ! ");
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
		} else if (token == "location") {
			inLocationBlock = true;
			iss >> currentLocationPath;
			currentLocation = LocationConfig();
		} else if (token == "}") {
			if (inLocationBlock) {
				inLocationBlock = false;
				currentServer.locations[currentLocationPath] = currentLocation;
			} else if (inServerBlock) {
				inServerBlock = false;
				servers.push_back(currentServer);
			}
		} else if (inServerBlock) {
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
			} else if (token == "client_max_body_size") {
				std::string sizeStr;
				iss >> sizeStr;
				currentServer.client_max_body_size = parseSize(sizeStr);
			}
		} else if (inLocationBlock) {
			if (token == "root") {
				iss >> currentLocation.root;
			} else if (token == "index") {
				iss >> currentLocation.index;
			} else if (token == "autoindex") {
				std::string value;
				iss >> value;
				currentLocation.autoindex = (value == "on");
			} else if (token == "limit_except") {
				std::string method;
				while (iss >> method) {
					currentLocation.limit_except.push_back(method);
				}
			} else if (token == "allow_upload") {
				std::string value;
				iss >> value;
				currentLocation.allow_upload = (value == "on");
			} else if (token == "upload_store") {
				iss >> currentLocation.upload_store;
			} else if (token == "cgi_pass") {
				iss >> currentLocation.cgi_pass;
			}
		}
	}
	return servers;
}

size_t ConfigParser::parseSize(const std::string& sizeStr) const {
    size_t size = 0;
    char unit = sizeStr[sizeStr.size() - 1];

    if (unit == 'M') {
        size = atoi(sizeStr.substr(0, sizeStr.size() - 1).c_str()) * 1024 * 1024;
    } else if (unit == 'K') {
        size = atoi(sizeStr.substr(0, sizeStr.size() - 1).c_str()) * 1024;
    } else {
        size = atoi(sizeStr.c_str());
    }

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