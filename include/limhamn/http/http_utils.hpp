/* limhamn::http::utils - Convenient utilities for HTTP requests and responses
 * Copyright (c) 2023-2025 Jacob Nilsson
 * Licensed under the MIT license
 *
 * Dependencies: OpenSSL
 * C++ version: >=17
 * File version: 0.1.0
 */
#pragma once

#include <string>
#include <unordered_map>
#include <openssl/evp.h>

#define LIMHAMN_HTTP_UTILS

/**
 * @brief  Namespace that contains all the networking related classes and functions.
 */
namespace limhamn::http::utils {
    /**
     * @brief  Enum class representing the protocol used in a URL
     */
    enum class protocol {
        http,
        https,
    };

    /**
     * @brief  Class representing a parsed URL
     */
    class url {
        public:
            std::string host{};
            std::string endpoint{};
            std::string query{};
            protocol protocol{protocol::http};
            int port{80};

            /**
             * @brief  Separate the components of a URL in the form of a string
             * @param  URL The URL to parse. The components can be accessed from the URL object.
             */
            void parse_url_from_string(const std::string& URL);
            /**
             * @brief  Assemble a URL from specified parts
             * @return Returns a full URL based on the parts
             */
            std::string assemble_url_from_parts() const;
    };

    struct multipart_file {
        std::string name{};
        std::string filename{};
        std::string path{};
        std::string sha256{};
        std::size_t size{};
    };

    /**
     * @brief Parse the request body into a map of fields.
     * @param body The request body.
     * @return std::unordered_map<std::string, std::string>
     */
    std::unordered_map<std::string, std::string> parse_fields(const std::string& body);
    /**
     * @brief Parse the query string into a map of fields.
     * @param url The URL to parse.
     * @return std::unordered_map<std::string, std::string>
     */
    std::unordered_map<std::string, std::string> parse_query_string(const std::string& url);
    /**
     * @brief  Function that replaces certain special characters with the matching HTML entity. Same behavior as PHP's htmlspecialchars function.
     * @param  str The string to replace in.
     * @return std::string
     */
    std::string htmlspecialchars(const std::string& str);
    /**
     * @brief  Function that replaces certain HTML entices with the matching special characters. Same behavior as PHP's htmlspecialchars_decode function.
     * @param  str The string to replace in.
     * @return std::string
     */
    std::string htmlspecialchars_decode(const std::string& str);
    /**
     * @brief  Function that URL encodes a string.
     * @param  str The string to encode.
     * @return std::string
     */
    std::string urlencode(const std::string& str);
    /**
     * @brief  Function that URL decodes a string.
     * @param  str The string to decode.
     * @return std::string
     */
    std::string urldecode(const std::string& str);
    /**
     * @brief  Function that removes single and double quotes from a std::string.
     * @param  str The string to replace in.
     * @return std::string
     */
    std::string remove_quotes(const std::string& str);
    /**
     * @brief  Function that generates a random string of a specified length.
     * @param  length The length of the string to generate.
     * @return std::string
     */
    std::string generate_random_string(const int length);
    /**
     * @brief  Function that parses a multipart body, getting file data.
     * @param  request The body to parse.
     * @param  format The format used to get the output file. %f is replaced by the file name, %h is replaced by the file name sha256 hash, %r is replaced by a random string.
     * @param  max_chunk_size The max size of each chunk. If the chunk is larger, the file is rejected.
     * @note   request must contain a Content-Type header. If you're using http_server.hpp, pass in the request.raw_body.
     * @return std::vector<snet::Utils::multipart_file>
     */
    std::vector<multipart_file> parse_multipart_form_file(const std::string& request, const std::string& format, const std::size_t max_chunk_size = 100000000);
    /**
     * @brief  Function that parses a multipart body, getting form data.
     * @param  request The body to parse.
     * @param  max_len Max length of each value.
     * @return std::unordered_map<std::string, std::string>
     * @note   request must NOT contain a Content-Type header. If you're using http_server.hpp, pass in the request.body.
     */
    std::unordered_map<std::string, std::string> parse_multipart_form_data(const std::string& request, const std::size_t max_len = 512);
    /**
     * @brief  Function that parses a filename and returns an appropriate MIME type for use in a Content-Type header.
     * @param  fn The file name to parse.
     * @return std::string
     */
    std::string get_appropriate_content_type(const std::string& fn);
    /**
     * @brief  Generate a sha256 hash from a string.
     * @param  data The data to hash.
     * @return std::string
     */
    std::string sha256hash(const std::string& data);
}

#ifdef LIMHAMN_HTTP_UTILS_IMPL
inline std::unordered_map<std::string, std::string> limhamn::http::utils::parse_fields(const std::string& _body) {
    std::string body = _body;

    if (!body.empty() && body.back() != '&') {
        body += "&";
    }

    std::unordered_map<std::string, std::string> ret{};
    std::size_t last_amp_pos = 0;
    std::size_t amp_pos = body.find('&');

    while (amp_pos != std::string::npos) {
        std::size_t equals_pos = body.find('=', last_amp_pos);

        if (equals_pos != std::string::npos && equals_pos < amp_pos) {
            std::string key = body.substr(last_amp_pos, equals_pos - last_amp_pos);
            std::string value = body.substr(equals_pos + 1, amp_pos - equals_pos - 1);
            ret[key] = value;
        }

        last_amp_pos = amp_pos + 1;
        amp_pos = body.find('&', last_amp_pos);
    }

    return ret;
}

inline std::unordered_map<std::string, std::string> limhamn::http::utils::parse_query_string(const std::string& url) {
    std::size_t pos = url.find('?');

    if (pos == std::string::npos) {
        return {};
    }

    std::string req = "&" + url.substr(pos + 1);

    return parse_fields(req);
}

inline std::unordered_map<std::string, std::string> limhamn::http::utils::parse_multipart_form_data(const std::string& request, const std::size_t max_len) {
    const auto split = [](const std::string& str, const std::string& delimiter) -> std::vector<std::string> {
        std::vector<std::string> tokens{};

        std::size_t start{0};
        std::size_t end{str.find(delimiter)};

        while (end != std::string::npos) {
            tokens.push_back(str.substr(start, end - start));
            start = end + delimiter.length();
            end = str.find(delimiter, start);
        }

        tokens.push_back(str.substr(start, end));

        return tokens;
    };

    const auto trim = [](const std::string& str) -> std::string {
        const char* whitespace = " \t\n\r\f\v";
        std::size_t start = str.find_first_not_of(whitespace);
        std::size_t end = str.find_last_not_of(whitespace);

        return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
    };

    std::string boundary{};
    const std::size_t boundary_pos{request.find("boundary=")};

    if (boundary_pos != std::string::npos) {
        const std::size_t boundary_end{request.find("\r\n", boundary_pos)};
        boundary = "--" + request.substr(boundary_pos + 9, boundary_end - boundary_pos - 9);
    }

    if (boundary.empty()) {
        return {};
    }

    const std::vector<std::string> parts{split(request, boundary)};

    std::unordered_map<std::string, std::string> ret{};
    for (const auto& part : parts) {
        if (part.empty() || part == "--\r\n") {
            continue;
        }

        const std::size_t header_end{part.find("\r\n\r\n")};
        if (header_end == std::string::npos) {
            continue;
        }

        const std::string headers{part.substr(0, header_end)};
        const std::string content{part.substr(header_end + 4)};

        if (headers.find("filename=") != std::string::npos) {
            continue;
        }

        std::string key{};
        std::string value{};

        std::size_t name_pos = headers.find("name=");
        if (name_pos != std::string::npos) {
            std::size_t name_start = headers.find('"', name_pos) + 1;
            std::size_t name_end = headers.find('"', name_start);

            if (name_end - name_start > max_len) {
                continue;
            }

            key = headers.substr(name_start, name_end - name_start);
        }

        value = trim(content);

        if (value.length() > max_len) {
            continue;
        }

        if (!key.empty()) {
            ret[key] = value;
        }
    }

    return ret;
}

inline std::vector<limhamn::http::utils::multipart_file> limhamn::http::utils::parse_multipart_form_file(const std::string& request, const std::string& format, const std::size_t max_chunk_size) {
    const auto split = [](const std::string& str, const std::string& delimiter) -> std::vector<std::string> {
        std::vector<std::string> tokens{};
        std::size_t start{0};
        std::size_t end{str.find(delimiter)};

        while (end != std::string::npos) {
            tokens.push_back(str.substr(start, end - start));
            start = end + delimiter.length();
            end = str.find(delimiter, start);
        }

        tokens.push_back(str.substr(start, end));

        return tokens;
    };

    const auto trim = [](const std::string& str) -> std::string {
        const char* whitespace{" \t\n\r\f\v"};
        const std::size_t start{str.find_first_not_of(whitespace)};
        const std::size_t end{str.find_last_not_of(whitespace)};

        return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
    };

    std::string boundary{};
    const std::size_t boundary_pos{request.find("boundary=")};
    if (boundary_pos != std::string::npos) {
        const std::size_t boundary_end{request.find("\r\n", boundary_pos)};

        if (boundary_end == std::string::npos) {
            return {};
        }

        boundary = "--" + request.substr(boundary_pos + 9, boundary_end - boundary_pos - 9);
    }

    if (boundary.empty()) {
        return {};
    }

    const std::vector<std::string> parts{split(request, boundary)};
    std::vector<multipart_file> ret;
    for (const auto& part : parts) {
        if (part.empty() || part == "--\r\n") {
            continue;
        }

        const std::size_t end{part.find("\r\n\r\n")};
        if (end == std::string::npos) {
            continue;
        }

        const std::string headers{part.substr(0, end)};
        const std::string content{part.substr(end + 4)};

        multipart_file file{};
        const std::size_t pos{headers.find("filename=")};

        if (pos != std::string::npos && (headers.at(pos - 1) == ' ' || headers.at(pos - 1) == ';')) {
            const std::size_t filename_start{headers.find('"', pos) + 1};
            const std::size_t filename_end{headers.find('"', filename_start)};

            if (filename_start == std::string::npos || filename_end == std::string::npos) {
                continue;
            }

            file.filename = headers.substr(filename_start, filename_end - filename_start);
            file.filename.erase(std::remove_if(file.filename.begin(), file.filename.end(), [](const char c) {
                return c == '/' || c == '\\';
            }), file.filename.end());

            if (file.filename.empty()) {
                continue;
            }
        } else {
            continue;
        }

        std::size_t pos2{headers.find(" name=")};
        if (pos2 == std::string::npos) {
            pos2 = headers.find(";name=");
        }
        if (pos2 != std::string::npos) {
            std::size_t name_start{headers.find('"', pos2) + 1};
            std::size_t name_end{headers.find('"', name_start)};

            if (name_start == std::string::npos || name_end == std::string::npos) {
                continue;
            }

            file.name = headers.substr(name_start, name_end - name_start);
        } else {
            continue;
        }

        file.path = format;
        file.sha256 = limhamn::http::utils::sha256hash(file.name);

        while (file.path.find("%f") != std::string::npos) {
            file.path.replace(file.path.find("%f"), 2, file.name);
        }
        while (file.path.find("%h") != std::string::npos) {
            file.path.replace(file.path.find("%h"), 2, file.sha256);
        }
        while (file.path.find("%r") != std::string::npos) {
            file.path.replace(file.path.find("%r"), 2, limhamn::http::utils::generate_random_string(64));
        }

        std::ofstream of{file.path};

        if (!of.is_open()) {
            continue;
        }

        std::string data{trim(content)};
        file.size = data.size();

        if (file.size > max_chunk_size) {
            of.close();
            std::filesystem::remove(file.path);
            continue;
        }

        of << data;
        of.close();

        if (!file.name.empty() && !data.empty()) {
            bool new_file{true};
            for (auto& it : ret) {
                if (it.name == file.name) {
                    it.size += file.size;
                    new_file = false;

                    break;
                }
            }

            if (new_file) {
                ret.push_back(file);
            }
        }
    }

    return ret;
}

inline std::string limhamn::http::utils::htmlspecialchars(const std::string& str) {
    std::string ret{str};

    for (std::size_t i{0}; i < ret.length(); i++) {
        if (ret.at(i) == '<') {
            ret.replace(i, 1, "&lt;");
        } else if (ret.at(i) == '>') {
            ret.replace(i, 1, "&gt;");
        } else if (ret.at(i) == '&') {
            ret.replace(i, 1, "&amp;");
        } else if (ret.at(i) == '"') {
            ret.replace(i, 1, "&quot;");
        } else if (ret.at(i) == '\'') {
            ret.replace(i, 1, "&apos;");
        } else if (ret.at(i) == '\\') {
            ret.replace(i, 1, "&bsol;");
        }
    }

    return ret;
}

inline std::string limhamn::http::utils::urldecode(const std::string& str) {
    std::string ret;
    ret.reserve(str.length());

    for (std::size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            std::istringstream iss(str.substr(i + 1, 2));
            int hex_value;
            if (iss >> std::hex >> hex_value) {
                ret += static_cast<char>(hex_value);
                i += 2;
            } else {
                ret += '%';
            }
        } else {
            ret += str[i];
        }
    }

    return ret;
}

inline std::string limhamn::http::utils::urlencode(const std::string& str) {
    std::ostringstream encoded;
    encoded.fill('0');
    encoded << std::hex;

    for (const auto& c : str) {
        if (isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        } else {
            encoded << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
        }
    }

    return encoded.str();
}

inline std::string limhamn::http::utils::htmlspecialchars_decode(const std::string& str) {
    std::string ret{str};

    for (std::size_t i{0}; i < ret.length(); i++) {
        if (ret.at(i) == '&') {
            if (ret.at(i + 1) == 'l' && ret.at(i + 2) == 't' && ret.at(i + 3) == ';') {
                ret.replace(i, 4, "<");
            } else if (ret.at(i + 1) == 'g' && ret.at(i + 2) == 't' && ret.at(i + 3) == ';') {
                ret.replace(i, 4, ">");
            } else if (ret.at(i + 1) == 'a' && ret.at(i + 2) == 'm' && ret.at(i + 3) == 'p' && ret.at(i + 4) == ';') {
                ret.replace(i, 5, "&");
            } else if (ret.at(i + 1) == 'q' && ret.at(i + 2) == 'u' && ret.at(i + 3) == 'o' && ret.at(i + 4) == 't' && ret.at(i + 5) == ';') {
                ret.replace(i, 6, "\"");
            } else if (ret.at(i + 1) == 'a' && ret.at(i + 2) == 'p' && ret.at(i + 3) == 'o' && ret.at(i + 4) == 's' && ret.at(i + 5) == ';') {
                ret.replace(i, 6, "'");
            } else if (ret.at(i + 1) == 'b' && ret.at(i + 2) == 's' && ret.at(i + 3) == 'o' && ret.at(i + 4) == 'l' && ret.at(i + 5) == ';') {
                ret.replace(i, 6, "\\");
            }
        }
    }

    return ret;
}

inline std::string limhamn::http::utils::remove_quotes(const std::string& str) {
    std::string ret{str};

    ret.erase(std::remove_if(ret.begin(), ret.end(), [](const char c) {
        return c == '\'' || c == '"';
    }), ret.end());

    return ret;
}

inline std::string limhamn::http::utils::generate_random_string(const int length) {
    static constexpr char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    static constexpr size_t charset_size = sizeof(charset) - 1;
    static std::mt19937 generator(std::random_device{}());

    std::uniform_int_distribution<> distribution(0, charset_size - 1);

    std::string str(length, 0);

    std::generate_n(str.begin(), length, [&distribution]() { return charset[distribution(generator)]; });

    return str;
}


inline std::string limhamn::http::utils::get_appropriate_content_type(const std::string& fn) {
    std::size_t pos = fn.find_last_of('.');
    if (pos == std::string::npos) {
        return "application/octet-stream";
    }

    std::string file = fn.substr(pos);

    static const std::unordered_map<std::string, std::string> content_type_map {
        {".aac", "audio/aac"},
        {".abw", "application/x-abiword"},
        {".apng", "image/apng"},
        {".arc", "application/x-freearc"},
        {".avif", "image/avif"},
        {".avi", "video/x-msvideo"},
        {".azw", "application/vnd.amazon.ebook"},
        {".bin", "application/octet-stream"},
        {".bmp", "image/bmp"},
        {".bz", "application/x-bzip"},
        {".bz2", "application/x-bzip2"},
        {".cda", "application/x-cdf"},
        {".csh", "application/x-csh"},
        {".css", "text/css"},
        {".csv", "text/csv"},
        {".doc", "application/msword"},
        {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
        {".eot", "application/vnd.ms-fontobject"},
        {".epub", "application/epub+zip"},
        {".gz", "application/gzip"},
        {".gif", "image/gif"},
        {".htm", "text/html"},
        {".html", "text/html"},
        {".ico", "image/vnd.microsoft.icon"},
        {".ics", "text/calendar"},
        {".jar", "application/java-archive"},
        {".jpeg", "image/jpeg"},
        {".jpg", "image/jpeg"},
        {".js", "text/javascript"},
        {".json", "application/json"},
        {".jsonld", "application/ld+json"},
        {".mid", "audio/x-midi"},
        {".midi", "audio/midi"},
        {".mjs", "text/javascript"},
        {".mp3", "audio/mpeg"},
        {".mp4", "video/mp4"},
        {".flac", "audio/flac"},
        {".mpeg", "video/mpeg"},
        {".mpkg", "application/vnd.apple.installer+xml"},
        {".odp", "application/vnd.oasis.opendocument.presentation"},
        {".ods", "application/vnd.oasis.opendocument.spreadsheet"},
        {".odt", "application/vnd.oasis.opendocument.text"},
        {".oga", "audio/ogg"},
        {".ogv", "video/ogg"},
        {".ogx", "application/ogg"},
        {".opus", "audio/ogg"},
        {".otf", "font/otf"},
        {".png", "image/png"},
        {".pdf", "application/pdf"},
        {".php", "application/x-httpd-php"},
        {".ppt", "application/vnd.ms-powerpoint"},
        {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
        {".rar", "application/vnd.rar"},
        {".rtf", "application/rtf"},
        {".sh", "application/x-sh"},
        {".svg", "image/svg+xml"},
        {".tar", "application/x-tar"},
        {".tif", "image/tiff"},
        {".tiff", "image/tiff"},
        {".ts", "video/mp2t"},
        {".ttf", "font/ttf"},
        {".txt", "text/plain"},
        {".vsd", "application/vnd.visio"},
        {".wav", "audio/wav"},
        {".weba", "audio/webm"},
        {".webm", "video/webm"},
        {".webp", "image/webp"},
        {".woff", "font/woff"},
        {".woff2", "font/woff2"},
        {".xhtml", "application/xhtml+xml"},
        {".xls", "application/vnd.ms-excel"},
        {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
        {".xml", "application/xml"},
        {".xul", "application/vnd.mozilla.xul+xml"},
        {".zip", "application/zip"},
        {".3gp", "video/3gpp"},
        {".3g2", "video/3gpp2"},
        {".7z", "application/x-7z-compressed"},
    };

    if (content_type_map.find(file) != content_type_map.end()) {
        return content_type_map.at(file);
    } else {
        return "application/octet-stream";
    }
}

inline std::string limhamn::http::utils::sha256hash(const std::string& data) {
    std::string ret{};

    EVP_MD_CTX* context = EVP_MD_CTX_new();

    if (context != nullptr) {
        if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr)) {
            if (EVP_DigestUpdate(context, data.c_str(), data.length())) {
                unsigned char hash[EVP_MAX_MD_SIZE];
                unsigned int len{0};

                if (EVP_DigestFinal_ex(context, hash, &len)) {
                    std::stringstream ss{};

                    for (unsigned int i{0}; i < len; ++i) {
                        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
                    }

                    ret = ss.str();
                }
            }
        }

        EVP_MD_CTX_free(context);
    }

    return ret;
}

inline void limhamn::http::utils::url::parse_url_from_string(const std::string& URL) {
    std::string url{URL};
    std::size_t pos{url.find("https://")};

    if (pos == std::string::npos) {
        this->protocol = limhamn::http::utils::protocol::http;
    } else {
        this->protocol = limhamn::http::utils::protocol::https;
    }

    pos = url.find(":");

    if (pos != std::string::npos && url.substr(pos, 3) == "://") {
        url = url.substr(pos + 3);
    }

    pos = url.find(":");

    if (pos != std::string::npos) {
        std::size_t pos2{url.find("/", pos)};
        this->host = url.substr(0, pos);
        this->port = std::stoi(url.substr(pos + 1, pos2 - pos - 1));
        url = url.substr(pos2);
    } else {
        std::size_t pos2{url.find("/")};
        this->host = url.substr(0, pos2);
        this->port = this->protocol == limhamn::http::utils::protocol::http ? 80 : 443;
        url = url.substr(pos2);
    }

    pos = url.find("?");

    if (pos == std::string::npos) {
        this->endpoint = url;
    } else {
        this->endpoint = url.substr(0, pos);
        this->query = url.substr(pos);
    }
}

inline std::string limhamn::http::utils::url::assemble_url_from_parts() const {
    std::string ret{};

    if (protocol == limhamn::http::utils::protocol::https) {
        ret += "https://" + host;
    } else {
        ret += "http://" + host; //NOLINT
    }

    if (!(protocol == limhamn::http::utils::protocol::https && port == 443) &&
        !(protocol == limhamn::http::utils::protocol::http && port == 80)) {
        ret += ":" + std::to_string(port);
        }

    ret += endpoint + query;

    return ret;
}
#endif