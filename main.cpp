/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/02 22:15:29 by smagniny          #+#    #+#             */
/*   Updated: 2024/11/02 23:58:38 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Http/Http.hpp"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "\n\n\tUsage: " << argv[0] << " <config_file> !!\n\n" << std::endl;
        return 1;
    }

    Http    http_launcher;

    http_launcher.setupSignalHandlers(&http_launcher);
    http_launcher.configure(argv[1]);
    http_launcher.launch_all();
    http_launcher.free_servers();

    // try{
        

    // } catch (const std::exception& e) {
        
    // }
}