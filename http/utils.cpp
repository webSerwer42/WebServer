/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agorski <agorski@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/01 14:46:44 by antek             #+#    #+#             */
/*   Updated: 2025/12/05 15:46:20 by agorski          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

long long parseContentLength(const std::string& contentLength) {
    
    std::stringstream ss(contentLength);
    long long value;

    ss >> value;

    // Sprawdzenie, czy konwersja się powiodła i czy cały string został zużyty.
    // ss.fail() - błąd konwersji (np. nie-cyfry)
    // !ss.eof() - po liczbie wystąpiły dodatkowe znaki (np. "123a")
    if (ss.fail() || !ss.eof()) {
        throw std::invalid_argument("default.conf argument client_max_body_size is invalid " + std::string(strerror(errno)));
        // Błąd: nieprawidłowa wartość lub wartość poza zakresem
    }
    return value;
}
