#include "Handlers.hpp"

std::string     readFile(const std::string& path)
{
    std::ifstream file(path.c_str());
    if (!file.is_open())
    {
        std::ifstream error_file("/var/www/error-pages/500.html");
        if (!error_file.is_open())
            return "Internal Error";
        std::stringstream buffer;
        buffer << error_file.rdbuf();
        return buffer.str();
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string                         urlDecode(const std::string &str) 
{
    std::string result;
    char ch;
    int i, ii;
    for (i = 0; i < str.length(); i++) {
        if (str[i] == '%') {
            sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            result += ch;
            i = i + 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

std::string                         escapeHtml(const std::string& data) {
    std::string result;
    for (size_t i = 0; i < data.length(); ++i) {
        unsigned char c = data[i];
        switch (c) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&#39;"; break;
            // Acentos y caracteres especiales
            case 0xC3:
                if (i + 1 < data.length()) {
                    unsigned char nextChar = data[i + 1];
                    switch (nextChar) {
                        case 0xA1: result += "&aacute;"; break;  // á
                        case 0xA9: result += "&eacute;"; break;  // é
                        case 0xAD: result += "&iacute;"; break;  // í
                        case 0xB3: result += "&oacute;"; break;  // ó
                        case 0xBA: result += "&uacute;"; break;  // ú
                        case 0xB1: result += "&ntilde;"; break;  // ñ
                        case 0x91: result += "&Ntilde;"; break;  // Ñ
                        default: result += c; break;
                    }
                    i++;  // Salta el siguiente byte del carácter multibyte
                } else {
                    result += c;
                }
                break;
            default: result += c; break;
        }
    }
    return result;
}