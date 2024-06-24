/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <smagniny@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 00:28:18 by smagniny          #+#    #+#             */
/*   Updated: 2024/06/17 17:44:42 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <poll.h>
# include <cerrno>
# include <cstring>
# include <map>
# include "MotherSocket.hpp"
# include "ServerConfig.hpp"

class Server: public MotherSocket
{
    private:
		std::vector<struct pollfd> fds;  // Dynamic array of pollfd structures
        char    buffer[20000];

        void    accepter();
        void    reader(int client_socket);
        void    writer(int client_socket);

    public:
        Server();
        Server(const ServerConfig& serverConfig);
        ~Server();

        void    launch();
        void    handleRequest(int client_socket, const std::string& request);
};

#endif