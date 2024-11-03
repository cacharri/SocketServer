/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 01:50:01 by smagniny          #+#    #+#             */
/*   Updated: 2024/11/03 02:57:53 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Client/Client.hpp"


Client::Client(ClientInfo* info) 
    : clientFd(info->pfd.fd)
    , session_info(info)
    , buffer(info->client_max_body_size)
{
    ReadFromConexion();
    response = new Response();
}

Client::~Client()
{
    delete request;
    delete response;
}

Request*    Client::getRequest()
{ 
    return request;
}

Response*   Client::getResponse()
{ 
    return response;
}
    
bool        Client::shouldKeepAlive() const 
{ 
    return session_info->keepAlive;
}
    
time_t      Client::getLastActivity() const 
{ 
    return session_info->lastActivity;
}

void    Client::ReadFromConexion()
{
    try {
        request = new Request(*session_info);
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

