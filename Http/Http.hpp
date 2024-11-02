/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Http.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/02 22:10:25 by smagniny          #+#    #+#             */
/*   Updated: 2024/11/02 23:42:36 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_HPP
#define HTTP_HPP

#include "../Server/Server.hpp"
#include "../Config/ConfigParser.hpp"
#include "../Logger/Logger.hpp"
#include <iostream>
#include <list>
#include <stdexcept>
#include <signal.h>

class Http
{
private:
    std::list<Server*>  servers;

    Http(const Http& copy);
    Http&   operator=(const Http& assign);
public:
    Http();
    ~Http();

    class ServerError: public std::exception
    {
        private:
            std::string error_string;
        public:
            ServerError(const std::string& error);
            virtual const char* what() const throw();
            virtual ~ServerError() throw();
    };

    void    configure(const std::string&  configfile);
    void    launch_all();
    void    free_servers();
    void    quitSignal();
    void    setupSignalHandlers(Http* http);

};

#endif