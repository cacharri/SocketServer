/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Smagniny <santi.mag777@gmail.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 15:44:42 by Smagniny          #+#    #+#             */
/*   Updated: 2024/09/14 21:00:39 by Smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
class Logger
{
private:
    static Logger*  instance;
    std::ofstream   logFile;
    std::string     logFileName;

    Logger(); // Private constructor for singleton pattern
    Logger(const Logger&); // Prevent copy construction
    Logger& operator=(const Logger&); // Prevent assignment

public:
    static Logger* getInstance();
    ~Logger();

    void log(const std::string& message, const char* file, int line);
    void logException(const std::exception& e, const char* file, int line);

    // MACROS
    #define LOG(message) Logger::getInstance()->log(message, __FILE__, __LINE__)
    #define LOG_EXCEPTION(e) Logger::getInstance()->logException(e, __FILE__, __LINE__)
};

#endif