/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Smagniny <santi.mag777@gmail.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 15:57:54 by Smagniny          #+#    #+#             */
/*   Updated: 2024/09/14 21:02:45 by Smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

Logger* Logger::instance = NULL;

Logger* Logger::getInstance() {
    if (instance == NULL) {
        instance = new Logger();
    }
    return instance;
}

Logger::Logger() {
    logFileName = "webserver.log";
    logFile.open(logFileName.c_str(), std::ios::app);
    
}

Logger::~Logger() {
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


void Logger::log(const std::string& message, const char* file, int line) {
    time_t now = time(0);
    char* dt = ctime(&now);
    std::string timestamp(dt);
    timestamp = timestamp.substr(0, timestamp.length() - 1); // Remove newline
    
    std::string line_number;
    std::stringstream out;
    out << line;
    line_number = out.str();
    std::string logMessage = "[ " + timestamp + " ]  " + "[ " + file +" ]  " + "[ " + line_number +" ] " + "[ "+ message +" ]" + "\n";

    if (logFile.is_open()) {
        logFile << logMessage;
        logFile.flush();
    }
    std::cout << logMessage;
}

void Logger::logException(const std::exception& e, const char* file, int line) 
{
    log(e.what(), file, line);
    throw e;
}