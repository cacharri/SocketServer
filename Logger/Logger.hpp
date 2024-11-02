#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>

// Codes couleur ANSI
#define GREY    "\033[90m"
#define RED     "\033[31m"
#define RESET   "\033[0m"

class Logger
{
private:
    static Logger*  instance;
    std::ofstream   logFile;
    std::string     logFileName;

    Logger();
    Logger(const Logger&);
    Logger& operator=(const Logger&);

public:
    static Logger* getInstance();
    ~Logger();

    void logINFO(const char* file, int line, const std::string& message);
    void log(const char* file, int line, const std::string& message);
    void logException(const std::exception& e, const char* file, int line);
};

#define LOG_INFO(msg) Logger::getInstance()->logINFO(__FILE__, __LINE__, msg)
#define LOG(msg) Logger::getInstance()->log(__FILE__, __LINE__, msg)
#define LOG_EXCEPTION(e) Logger::getInstance()->logException(e, __FILE__, __LINE__)

#endif 