/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <smagniny@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 01:50:01 by smagniny          #+#    #+#             */
/*   Updated: 2024/11/21 18:36:19 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Client/Client.hpp"


Client::Client(ClientInfo* info) 
    : clientFd(info->pfd.fd)
    , session_info(info)
{

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

void        Client::setLastActivity() const 
{ 
    session_info->lastActivity = time(NULL);
}

bool Client::HandleConnection()
{
    response = new Response();
    try {
        request = new Request(*session_info);
        if (request->readData((*session_info).pfd.fd, (*session_info).client_max_body_size) == false)
            return false;
    }
    catch (const std::exception& RequestError){
        LOG(RequestError.what());
        return false;
    }

    return true;
}

