#include "Logger.hpp"

Logger* Logger::instance = NULL;

Logger* Logger::getInstance()
{
    if (instance == NULL)
        instance = new Logger();
    return instance;
}

Logger::Logger()
{
    logFileName = "webserver.log";
    logFile.open(logFileName.c_str(), std::ios::app);
}

Logger::~Logger()
{
    if (logFile.is_open())
    {
        logFile.close();
    }
    if (instance)
    {
        delete instance;
        instance = NULL;
    }
}

void Logger::logINFO(const char* file, int line, const std::string& message)
{
    time_t now = time(0);
    char* dt = ctime(&now);
    std::string timestamp(dt);
    timestamp = timestamp.substr(0, timestamp.length() - 1);

    std::ostringstream lineStr;
    lineStr << line;

    std::cout << GREEN << "[ " << timestamp << " ] [ " << file << ":" 
              << lineStr.str() << " ] " << message << RESET << std::endl;
}

void Logger::log(const char* file, int line, const std::string& message)
{
    time_t now = time(0);
    char* dt = ctime(&now);
    std::string timestamp(dt);
    timestamp = timestamp.substr(0, timestamp.length() - 1);

    std::ostringstream lineStr;
    lineStr << line;

    std::string logMessage = "[ " + timestamp + " ] [ " + file + ":" + 
                           lineStr.str() + " ] " + message + "\n";

    if (logFile.is_open()) {
        logFile << logMessage;
        logFile.flush();
    }
    std::cout << RED << logMessage << RESET;
}

void Logger::logException(const std::exception& e, const char* file, int line)
{
    log(file, line, std::string("Exception: ") + e.what());
    throw e;
} 