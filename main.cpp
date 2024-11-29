/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <smagniny@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/02 22:15:29 by smagniny          #+#    #+#             */
/*   Updated: 2024/11/29 15:40:21 by smagniny         ###   ########.fr       */
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
    if (http_launcher.configure(argv[1]) == false)
        return (1);
    http_launcher.launch_all();
}