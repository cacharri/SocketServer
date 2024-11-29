/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Http.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <smagniny@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/02 22:10:25 by smagniny          #+#    #+#             */
/*   Updated: 2024/11/29 14:30:21 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_HPP
#define HTTP_HPP

#include "../Server/Server.hpp"
#include "../Config/ConfigParser.hpp"
#include <list>
#include <signal.h>

class Http
{
private:
    std::list<Server*>  servers;

    Http(const Http& copy);
    Http&   operator=(const Http& assign);
    void    loadServers(std::vector<pollfd>& master_fds, size_t& num_servers);
    void    loadNewConnections(size_t& total_clients, std::vector<pollfd>& master_fds);
    void    CheckUserConnected(size_t& previous_clients_size, std::vector<pollfd>& master_fds);
    void    CGI_events(size_t& cgi_index, std::vector<pollfd>& master_fds);
    void    Clients_events(size_t& fd_index, std::vector<pollfd>& master_fds);
    void    Server_events(size_t& fd_index, std::vector<pollfd>& master_fds);

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