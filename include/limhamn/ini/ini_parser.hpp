/* limhamn::ini - A simple INI parser for C++
 * Copyright (c) 2023-2025 Jacob Nilsson
 * Licensed under the MIT license
 *
 * Dependencies: None
 * C++ version: >=17
 * File version: 0.1.0
 */

#pragma once

#include <string>
#ifdef LIMHAMN_INI_PARSER_IMPL
#include <fstream>
#include <sstream>
#endif
#include <unordered_map>

#define LIMHAMN_INI_PARSER

/**
 * @brief A class to manage INI files
 */
namespace limhamn::ini {
    using key = std::string;
    using value = std::string;
    using header_value = std::unordered_map<key, value>;
    using header_key = std::string;
    using config = std::unordered_map<header_key, header_value>;

    /**
     * @brief A class to manage INI files
     */
    class ini_parser {
        config parsed_map{};

        /**
         * @brief Parse the INI data
         * @param data INI data
         */
        void parse(const std::string& data);
    public:
        /**
         * @brief Construct a new ini object
         */
        ini_parser() = default;
        /**
         * @brief Destroy the ini object
         */
        ~ini_parser() = default;
        /**
         * @brief Construct a new ini object
         * @param data INI data
         * @param is_file Is the data a file
         */
        explicit ini_parser(const std::string& data, bool is_file = false);
        /**
         * @brief Load INI data
         * @param data INI data
         * @param is_file Is the data a file
         */
        void load(const std::string& data, bool is_file = false);
        /**
         * @brief Get a value from the INI data
         * @param header Header
         * @param key Key
         * @return Value
         */
        [[nodiscard]] value& get(const std::string& header, const std::string& key);
        /**
         * @brief Get the INI data
         * @return INI data
         */
        [[nodiscard]] config get_data() const;
        /**
         * @brief Get a header from the INI data
         * @param header Header
         * @return HeaderValue
         */
        [[nodiscard]] header_value& get_header(const std::string& header);
        /**
         * @brief Get the INI data as a string
         * @return INI data as a string
         */
        [[nodiscard]] std::string to_string();
        /**
         * @brief Save the INI data to a file
         * @param file File
         */
        void save(const std::string& file);
        /**
         * @brief Set a value in the INI data
         * @param header Header
         * @param key Key
         * @param value Value
         */
        void set(const std::string& header, const std::string& key, const std::string& value);
        /**
         * @brief Get a header from the INI data
         * @param header Header
         * @return header_value
         */
        [[nodiscard]] header_value& operator[](const std::string& header);
    };
}  // namespace limhamn::ini

#ifdef LIMHAMN_INI_PARSER_IMPL
inline void limhamn::ini::ini_parser::parse(const std::string& data) {
    parsed_map.clear();

    if (data.empty()) {
        throw std::invalid_argument("data is empty");
    }

    std::stringstream ss{data};
    std::string it{};
    std::string current_header{};
    while (std::getline(ss, it)) {
        it.erase(std::remove_if(it.begin(), it.end(), ::isspace), it.end());
        if (it.empty() || it.at(0) == ';' || it.at(0) == '#') {
            continue;
        }

        // check if it's a header
        if (it.at(0) == '[' && it.at(it.size() - 1) == ']') {
            current_header = it.substr(1, it.size() - 2);
            continue;
        }

        if (current_header.empty()) {
            continue;
        }

        // find = and split
        auto pos = it.find('=');
        if (pos + 1 == std::string::npos) {
            continue;
        }

        std::string key = it.substr(0, pos);
        std::string value = it.substr(pos + 1);

        pos = value.find(';');
        if (pos != std::string::npos) {
            if (pos - 1 != std::string::npos && value.at(pos - 1) != '\\') {
                value = value.substr(0, pos);
            }
        }

        pos = value.find('#');
        if (pos != std::string::npos) {
            if (pos - 1 != std::string::npos && value.at(pos - 1) != '\\') {
                value = value.substr(0, pos);
            }
        }
        if (value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        }

        parsed_map[current_header][key] = value;
    }
}

inline limhamn::ini::ini_parser::ini_parser(const std::string& data, bool is_file) {
    this->load(data, is_file);
}

inline void limhamn::ini::ini_parser::load(const std::string& data, bool is_file) {
    this->parsed_map.clear();

    std::string buffer{};

    if (is_file) {
        std::ifstream file(data);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                buffer += line + "\n";
            }
            file.close();
        }
    }

    parse(is_file ? buffer : data);
}

[[nodiscard]] inline limhamn::ini::value& limhamn::ini::ini_parser::get(const std::string& header, const std::string& key) {
    if (header.empty()) {
        throw std::invalid_argument("header is empty; call get_data instead");
    }

    if (key.empty()) {
        throw std::invalid_argument("key is empty; call get_header instead");
    }

    if (parsed_map.find(header) == parsed_map.end()) {
        throw std::invalid_argument("header not found");
    }

    return parsed_map[header][key];
}

[[nodiscard]] inline limhamn::ini::config limhamn::ini::ini_parser::get_data() const {
    return parsed_map;
}

[[nodiscard]] inline limhamn::ini::header_value& limhamn::ini::ini_parser::get_header(const std::string& header) {
    if (header.empty()) {
        throw std::invalid_argument("header is empty");
    }

    if (parsed_map.find(header) == parsed_map.end()) {
        parsed_map[header] = {};
    }

    return parsed_map[header];
}

[[nodiscard]] inline std::string limhamn::ini::ini_parser::to_string() {
    std::string ret{};
    for (const auto& [header, values] : parsed_map) {
        if (values.empty() || header.empty()) {
            continue;
        }

        ret += "[" + header + "]\n";

        for (const auto& [key, value] : values) {
            ret += key;
            ret += "=" + value + "\n";
        }

        ret += "\n";
    }
    return ret;
}

inline void limhamn::ini::ini_parser::save(const std::string& file) {
    std::ofstream out{file};
    if (out.is_open()) {
        out << to_string();
        out.close();
    } else {
        throw std::runtime_error("could not open file for writing");
    }
}

inline void limhamn::ini::ini_parser::set(const std::string& header, const std::string& key, const std::string& value) {
    if (header.empty()) {
        throw std::invalid_argument("header is empty");
    }
    if (key.empty()) {
        throw std::invalid_argument("key is empty");
    }
    if (value.empty()) {
        parsed_map[header].erase(key);
        return;
    }
    parsed_map[header][key] = value;
}

[[nodiscard]] inline limhamn::ini::header_value& limhamn::ini::ini_parser::operator[](const std::string& header) {
    return get_header(header);
}
#endif
