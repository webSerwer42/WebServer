#include "stringUtils.hpp"

// Funkcja zamieniająca polskie znaki diakrytyczne (UTF-8) na ich odpowiedniki ASCII
std::string normalizePolishChars(const std::string& text) {
    std::string result;
    result.reserve(text.length());

    for (size_t i = 0; i < text.length(); ++i) {
        unsigned char c1 = text[i];
        if (c1 < 128) { // Zwykły znak ASCII
            result += c1;
        } else {
            // Sprawdzamy, czy to dwubajtowy znak UTF-8
            if (i + 1 < text.length()) {
                unsigned char c2 = text[i + 1];
                // Sprawdzamy konkretne polskie znaki
                if (c1 == 0xC4 && c2 == 0x85) result += 'a'; // ą
                else if (c1 == 0xC4 && c2 == 0x87) result += 'c'; // ć
                else if (c1 == 0xC4 && c2 == 0x99) result += 'e'; // ę
                else if (c1 == 0xC5 && c2 == 0x82) result += 'l'; // ł
                else if (c1 == 0xC5 && c2 == 0x84) result += 'n'; // ń
                else if (c1 == 0xC3 && c2 == 0xB3) result += 'o'; // ó
                else if (c1 == 0xC5 && c2 == 0x9B) result += 's'; // ś
                else if (c1 == 0xC5 && c2 == 0xBA) result += 'z'; // ź
                else if (c1 == 0xC5 && c2 == 0xBC) result += 'z'; // ż
                else if (c1 == 0xC4 && c2 == 0x84) result += 'A'; // Ą
                else if (c1 == 0xC4 && c2 == 0x86) result += 'C'; // Ć
                else if (c1 == 0xC4 && c2 == 0x98) result += 'E'; // Ę
                else if (c1 == 0xC5 && c2 == 0x81) result += 'L'; // Ł
                else if (c1 == 0xC5 && c2 == 0x83) result += 'N'; // Ń
                else if (c1 == 0xC3 && c2 == 0x93) result += 'O'; // Ó
                else if (c1 == 0xC5 && c2 == 0x9A) result += 'S'; // Ś
                else if (c1 == 0xC5 && c2 == 0xB9) result += 'Z'; // Ź
                else if (c1 == 0xC5 && c2 == 0xBB) result += 'Z'; // Ż
                i++; // Pomiń drugi bajt znaku
            }
        }
    }
    return result;
}

std::string wordToLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

// Funkcja pomocnicza do przetwarzania pobranego tokenu
void getWords(const std::string& inputToSplit, std::vector<std::string>& results, std::string words){
    if (inputToSplit.empty()) {
        return;
    }
    std::string word;
    
    for (std::string::const_iterator it = inputToSplit.begin(); it != inputToSplit.end(); ++it) {
        char c = *it;
        // Używamy unsigned char, aby poprawnie obsługiwać znaki spoza ASCII
        if (std::isalnum(static_cast<unsigned char>(c))) {
            word += c;
        } else if (words.find(c) != std::string::npos) { // Poprawiony warunek
            if (!word.empty()) {
                results.push_back(word);
                word.clear();
            }
            if (c != ' ') { // Pomijamy spacje
                results.push_back(std::string(1, c));
            }
        }
    }
    // Dodaj ostatnie słowo, jeśli istnieje
    if (!word.empty()) {
        results.push_back(word);
    }
}
// ============ ADDITIONAL HTTP UTILITY FUNCTIONS ============

std::string trim(const std::string& str) {
    if (str.empty())
        return str;
    
    size_t start = 0;
    size_t end = str.length();
    
    while (start < end && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }
    
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        --end;
    }
    
    return str.substr(start, end - start);
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

size_t stringToSize(const std::string& str) {
    if (str.empty())
        return 0;
    
    std::istringstream iss(str);
    size_t result;
    iss >> result;
    
    if (iss.fail())
        return 0;
    
    return result;
}
