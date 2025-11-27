/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agorski <agorski@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 14:34:32 by agorski           #+#    #+#             */
/*   Updated: 2025/11/27 15:05:20 by agorski          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>
#include <map>
#include <sstream>
#include <iostream>

class HttpError {
    private:
        std::map<int, std::string> errorMap;
        std::map<int, std::string> errorDescriptions;
        std::map<int, std::string> defaultErrorPages; // kod błędu → ścieżka do pliku
        
        void initializeErrorMaps();
        std::string generateHtmlBody(int code, const std::string& message, const std::string& description);
        
        // Metody pomocnicze dla custom pages
        bool fileExists(const std::string& path) const;
        bool isValidPath(const std::string& path) const;
        std::string readFileContent(const std::string& path) const;
        
    public:
        HttpError();
        
        // Wygeneruj kompletną odpowiedź HTTP z HTML
        std::string generateErrorResponse(int code);
        
        // Pobierz krótki opis błędu (np. "404 Not Found")
        std::string getErrorMessage(int code);
        
        // Pobierz pełny opis błędu
        std::string getErrorDescription(int code);
        
        // Wygeneruj kompletną odpowiedź HTTP z niestandardowym komunikatem
        std::string generateErrorResponse(int code, const std::string& customMessage);
        
        // Zarządzanie niestandardowymi stronami błędów
        void setDefaultErrorPage(int code, const std::string& filePath);
        void clearDefaultErrorPages();
        bool hasDefaultErrorPage(int code) const;
        std::string getDefaultErrorPagePath(int code) const;
};

#endif