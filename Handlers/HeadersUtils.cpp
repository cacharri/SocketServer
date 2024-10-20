/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeadersUtils.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/20 17:14:02 by smagniny          #+#    #+#             */
/*   Updated: 2024/10/20 22:25:28 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Handlers/HeaderAnalyzer.hpp"

#include <iostream>
#include <string>
#include <cctype>
#include <cstdlib>
#include <sstream>


// Es una cadena con solo numeros ?
bool isNumeric(const std::string &str) {
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (!isdigit(str[i]))
            return false;
    }
    return true;
}
// Es una cadena con solo letras
bool isAlphabetic(const std::string &str) {
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (!isalpha(str[i]))
            return false;
    }
    return true;
}


bool isValidIPv4(const std::string& ip)
{
    std::istringstream ss(ip);
    std::string token;
    int parts = 0;
    
    while (std::getline(ss, token, '.')) {
        if (!isNumeric(token))
            return false;
        int num = std::atoi(token.c_str());
        if (num < 0 || num > 255)
            return false;
        ++parts;
    }
    std::cout << "parts: " << parts << std::endl;

    return parts == 4;
}

// Comprueba que sea un dominio web valido
bool isValidDomain(const std::string &host)
{
    if (host.empty() || host.length() > 253)
        return false;

    std::istringstream iss(host);
    std::string label;
    bool first = true;

    while (std::getline(iss, label, '.')) {
        if (label.empty() || label.length() > 63)
            return false;

        if (first && label == "localhost")
            return true;
        if (first && !isAlphabetic(label)) //  checkear que el subdominio (www) sea solo letras sino false
            return false;
        if (!isalnum(label[0]) || !isalnum(label[label.length() - 1]))
            return false;
        for (size_t i = 0; i < label.length(); ++i) {
            if (!isalnum(label[i]) && label[i] != '-')
                return false;
        }
        first = false;
    }
    return !first; // Assurez-vous qu'il y avait au moins un label
}

// Comprueba Ipv6 [xxxx::xxxx::xxxx::xxxx]
bool isValidIPv6(const std::string& host)
{
    if (host.size() < 2 || host[0] == '[' || host[host.size() - 1] == ']')
        return true;
    
    return false;
}


bool isValidPort(const std::string &port) {
    if (port.empty())
        return true; // Si no hay puerto por defecto 80
    if (!isNumeric(port))
        return false;
    
    int port_num = std::atoi(port.c_str());
    
    return port_num >= 1 && port_num <= 65535;
}

// Checker del Header Host
bool isValidHost(const std::string &hostHeader)
{
    std::string host;
    std::string port;
    
    // Hay puerto definido ?
    size_t colonPos = hostHeader.find(':');
    if (colonPos != std::string::npos)
    {
        host = hostHeader.substr(0, colonPos);
        port = hostHeader.substr(colonPos + 1);
    } else
        host = hostHeader;
    
    // Comprobar si es Ipv4
    if (isValidIPv4(host))
        return isValidPort(port); // hay puerto y es valido ?
    if (isValidIPv6(host))
        return isValidPort(port);

    // Comprueba si es dominio web valido
    if (isValidDomain(host))
        return isValidPort(port);
    
    // si nada se cumple como valido chao papi
    return false;
}

bool isValidHostHeader(const std::string &hostHeader)
{
    std::istringstream iss(hostHeader);
    std::string host;
    bool valid = false;

    while (std::getline(iss, host, ','))
    {
        //borrar possibles espacios
        host.erase(0, host.find_first_not_of(" \t"));
        host.erase(host.find_last_not_of(" \t") + 1);

        if (isValidHost(host)) {
            valid = true;
            break; // Pillamos el primer host
        }
    }

    return valid;
}

// int main() {

//     std::string test1 = "www.example.com";
//     std::string test2 = "192.168.1.1";
//     std::string test3 = "[2001:db8::1]";
//     std::string test4 = "www.exa$mple.com"; // Incorrect
//     std::string test5 = "localhost:8080";
//     std::string test6 = "999.999.999.999"; // Incorrect IP

//     std::cout << "Test 1: " << isValidHost(test1) << std::endl;
//     std::cout << "Test 2: " << isValidHost(test2) << std::endl;
//     std::cout << "Test 3: " << isValidHost(test3) << std::endl;
//     std::cout << "Test 4: " << isValidHost(test4) << std::endl;
//     std::cout << "Test 5: " << isValidHost(test5) << std::endl;
//     std::cout << "Test 6: " << isValidHost(test6) << std::endl;

//     return 0;
// }

