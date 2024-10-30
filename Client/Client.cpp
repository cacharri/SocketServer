/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 01:50:01 by smagniny          #+#    #+#             */
/*   Updated: 2024/10/30 11:55:42 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Client/Client.hpp"


Client::Client(size_t fd, ClientInfo& info) 
    : clientFd(fd)
    , session_info(info)
    , buffer(info.client_max_body_size)
{
    ReadFromConexion();
    response = new Response();
}

Client::~Client()
{
    delete request;
    delete response;
}

void    Client::ReadFromConexion()
{
    try {
        request = new Request(clientFd, session_info);
    }
    catch (const std::exception& RequestError){
        throw ClientError(RequestError.what());
    }
    // try {
    //     request = new Request(clientFd, session_info);
    // }
    // catch (const std::exception& e) {
    //     response.setStatus(400, "Bad Request");
    //     response.setBody(e.what());
    //     keepAlive = false;
    // }
}

bool Client::HandleConnection()
{
    
    return true;
}

