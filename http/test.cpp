/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agorski <agorski@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 14:36:23 by agorski           #+#    #+#             */
/*   Updated: 2025/12/03 14:36:36 by agorski          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Http.hpp"

void Http::testResponseBuilder() {
//tempolary response for testing
    if (_s_requestData._path == "/favicon.ico") {
        _s_responseData._response = "HTTP/1.1 204 No Content\r\n"
                                   "Connection: close\r\n"
                                   "\r\n";
        return;
    }

    std::ostringstream html;

    html << "HTTP/1.1 200 OK\r\n"
         << "Content-Type: text/html; charset=UTF-8\r\n"
         << "Connection: close\r\n"
         << "\r\n"
         << "<!DOCTYPE html>\n"
         << "<html>\n"
         << "<head>\n"
         << "<meta charset=\"UTF-8\">\n"
         << "<title>" << 200 << " " << "OK" << "</title>\n"
         << "</head>\n"
         << "<body>\n"
         << "<h1>" << 200 << " " << "OK" << "</h1>\n"
         << "<p>" << "Request was successful." << "</p>\n"
         << "<p>Method: " << _s_requestData._method << "</p>\n"
         << "<p>Path: " << _s_requestData._path << "</p>\n"
         << "<p>Version: " << _s_requestData._httpVersion << "</p>\n"
         << "<p>Headers:</p>\n";
         for (std::map<std::string, std::string>::const_iterator it = _s_requestData._headers.begin(); 
              it != _s_requestData._headers.end(); ++it)
         {
             html << "<p><strong>" << it->first << ":</strong> " << it->second << "</p>\n";
             std::cout << "Header: " << it->first << " => " << it->second << std::endl;
         }
         // Wyświetl zawartość _myConfig
         html << "<h2>Location Config</h2>\n";
         html << "<p><strong>root:</strong> " << _myConfig.root << "</p>\n";
         html << "<p><strong>upload_dir:</strong> " << _myConfig.upload_dir << "</p>\n";
         html << "<p><strong>cgi_path:</strong> " << _myConfig.cgi_path << "</p>\n";
         html << "<p><strong>client_max_body_size:</strong> " << _myConfig.client_max_body_size << "</p>\n";
         html << "<p><strong>autoindex:</strong> " << (_myConfig.autoindex ? "true" : "false") << "</p>\n";

         html << "<p><strong>allow_methods:</strong></p>\n";
         for (size_t i = 0; i < _myConfig.allow_methods.size(); ++i)
             html << "<p> - " << _myConfig.allow_methods[i] << "</p>\n";

         html << "<p><strong>index:</strong></p>\n";
         for (size_t i = 0; i < _myConfig.index.size(); ++i)
             html << "<p> - " << _myConfig.index[i] << "</p>\n";

         html << "<p><strong>cgi_ext:</strong></p>\n";
         for (size_t i = 0; i < _myConfig.cgi_ext.size(); ++i)
             html << "<p> - " << _myConfig.cgi_ext[i] << "</p>\n";

         html << "<p><strong>error_pages:</strong></p>\n";
         for (std::map<int, std::string>::const_iterator it = _myConfig.error_pages.begin();
              it != _myConfig.error_pages.end(); ++it)
             html << "<p> - " << it->first << " => " << it->second << "</p>\n";
         html << "</body>\n"
              << "</html>";

    _s_responseData._response = html.str();
}
