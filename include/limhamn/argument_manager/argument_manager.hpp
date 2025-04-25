/* limhamn::argument_manager - Simple argument manager for C++20
 * Copyright (c) 2023-2025 Jacob Nilsson
 * Licensed under the MIT license
 *
 * Dependencies: None
 * C++ version: >=17
 * File version: 0.1.0
 */

#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#ifdef LIMHAMN_ARGUMENT_MANAGER_IMPL
#include <sstream>
#endif
#include <vector>

#define LIMHAMN_ARGUMENT_MANAGER

/**
 * @brief Namespace for the argument manager
 */
namespace limhamn::argument_manager {
    /**
     * @brief collection of arguments
     */
    struct collection {
        std::size_t index{0};
        std::vector<std::string> arguments;
    };

    /**
     * @brief Class for managing arguments
     */
    class argument_manager {
        public:
            /**
             * @brief Construct a new argument_manager object
             * @param argc Number of arguments
             * @param argv argument_managerument vector
             */
            explicit argument_manager(int argc, char** argv) : arguments(argv, argv + argc) {};
            /**
             * @brief Construct a new argument_manager object
             * @param args argument_managerument vector
             */
            explicit argument_manager(const std::vector<std::string>& args) : arguments(args) {};
            /**
             * @brief Add a flag to the argument manager
             * @param args Flag arguments
             * @param callback Callback function
             */
            void push_back(const std::string& args, const std::function<void(collection&)>& callback);
            /**
             * @brief Execute the argument manager
             * @param callback Callback function
             */
            void execute(const std::function<void(const std::string& arg)>& callback);
        private:
            std::vector<std::string> arguments;
            std::unordered_map<std::string, std::function<void(collection&)>> flags;
    };
}

#ifdef LIMHAMN_ARGUMENT_MANAGER_IMPL
inline void limhamn::argument_manager::argument_manager::push_back(const std::string& args, const std::function<void(collection&)>& callback) {
    std::stringstream ss(args);
    std::vector<std::string> tokens;

    for (std::string token; std::getline(ss, token, '|');) {
        tokens.push_back(token);
    }

    for (const auto& token : tokens) {
        flags[token] = callback;
    }
}

inline void limhamn::argument_manager::argument_manager::execute(const std::function<void(const std::string& arg)>& callback) {
    for (std::size_t i{1}; i < arguments.size(); ++i) {
        if (flags.find(arguments[i]) != flags.end()) {
            collection col;

            col.index = i;
            col.arguments = arguments;

            flags[arguments[i]](col);

            i = col.index;
        } else {
            callback(arguments.at(i));
        }
    }
}
#endif // LIMHAMN_ARGUMENT_MANAGER_IMPL
