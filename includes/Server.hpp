/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 00:28:18 by smagniny          #+#    #+#             */
/*   Updated: 2024/06/13 13:09:56 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <poll.h>
# include <cerrno>
# include <cstring>
# include "MotherSocket.hpp"

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
        ~Server();

        void    launch();
};

#endif