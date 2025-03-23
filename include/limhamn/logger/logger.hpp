/* limhamn::logger - Simple logger for C++
 * Copyright (c) 2023-2025 Jacob Nilsson
 * Licensed under the MIT license
 *
 * Dependencies: None
 * C++ version: >=17
 * File version: 0.1.0
 */

#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <ctime>

#define LIMHAMN_LOGGER

/**
 * @brief  logger namespace containing all the classes and functions for the logger.
 */
namespace limhamn::logger {
    /**
     * @brief  List of integers representing log types.
     */
    enum class type {
        access,
        error,
        warning,
        notice,
        undefined,
    };

    /**
     * @brief  List of integers representing success or failure.
     */
    enum class status {
        success,
        failure,
        undefined,
    };

    /**
     * @brief  List of integers representing known output streams
     */
    enum class stream {
        stdout,
        stderr,
        none,
    };

    using logger_status = status;
    using logger_error_type = type;
    using logger_stream = stream;
    using logger_file = std::string;
    using logger_boolean = bool;
    using logger_prefix = std::string;

    /**
     * @brief  Struct containing settings to initialize the logger with.
     */
    struct logger_properties {
        logger_boolean output_to_std{false};
        logger_boolean output_to_file{true};
        logger_stream stream{stream::stderr};
        logger_boolean log_date{true};
        logger_boolean log_access_to_file{true};
        logger_boolean log_error_to_file{true};
        logger_boolean log_warning_to_file{true};
        logger_boolean log_notice_to_file{true};
        logger_file access_log_file{"/var/log/limhamn/access.log"};
        logger_file error_log_file{"/var/log/limhamn/error.log"};
        logger_file warning_log_file{"/var/log/limhamn/warning.log"};
        logger_file notice_log_file{"/var/log/limhamn/notice.log"};
        logger_prefix access_log_prefix{"[ACCESS]: "};
        logger_prefix error_log_prefix{"[ERROR]: "};
        logger_prefix warning_log_prefix{"[WARNING]: "};
        logger_prefix notice_log_prefix{"[NOTICE]: "};
    };

    /**
     * @brief  Struct containing the return values of the logger.
     */
    struct logger_return {
        logger_error_type type{type::undefined};
        logger_status status{status::success};
        logger_stream stream{stream::none};
        std::string message{};
        std::string date{};
        std::string data{};
        std::string prefix{};
        std::string file{};
    };

    /**
     * @brief  Class that handles logging.
     */
    class logger {
            logger_properties prop{};
        public:
            /**
             * @brief  Constructor for the logger.
             * @param  prop: logger_properties struct containing the settings for the logger.
             */
            explicit logger(const logger_properties& prop);
            /**
             * @brief  Default constructor for the logger.
             */
            explicit logger() = default;
            /**
             * @brief  Default destructor for the logger.
             */
            ~logger() = default;

            /**
             * @brief  Writes data to the log, returning feedback.
             * @param  type: logger_error_type enum representing the type of log.
             * @param  data: std::string containing the data to log.
             * @return logger_return struct containing the return values of the logger.
             */
            logger_return write_to_log_f(logger_error_type type, const std::string& data) const noexcept; // NOLINT
            /**
             * @brief  Writes data to the log.
             * @param  type: logger_error_type enum representing the type of log.
             * @param  data: std::string containing the data to log.
             */
            void write_to_log(logger_error_type type, const std::string& data) const noexcept; // NOLINT
            /**
             * @brief  Overrides the properties of the logger.
             * @param  prop: logger_properties struct containing the settings for the logger.
             */
            void override_properties(const logger_properties& prop) noexcept;
            /**
             * @brief  Gets the properties of the logger.
             * @return logger_properties struct containing the settings for the logger.
             */
            [[nodiscard]] logger_properties get() noexcept;
    };
}

#ifdef LIMHAMN_LOGGER_IMPL
inline limhamn::logger::logger::logger(const logger_properties& prop) {
    this->prop = prop;
}

inline void limhamn::logger::logger::override_properties(const logger_properties& prop) noexcept {
    this->prop = prop;
}

inline limhamn::logger::logger_properties limhamn::logger::logger::get() noexcept {
    return this->prop;
}

inline void limhamn::logger::logger::write_to_log(const logger_error_type type, const std::string& data) const noexcept {
    static_cast<void>(write_to_log_f(type, data));
}

inline limhamn::logger::logger_return limhamn::logger::logger::write_to_log_f(const logger_error_type type, const std::string& data) const noexcept {
    std::string prefix{"[UNKNOWN]: "};
    std::string logfile{"logfile.txt"};
    logger_return ret{};

    if (type == type::warning) {
        prefix = prop.warning_log_prefix;
        logfile = prop.warning_log_file;
    } else if (type == type::error) {
        prefix = prop.error_log_prefix;
        logfile = prop.error_log_file;
    } else if (type == type::access) {
        prefix = prop.access_log_prefix;
        logfile = prop.access_log_file;
    } else if (type == type::notice) {
        prefix = prop.notice_log_prefix;
        logfile = prop.notice_log_file;
    } else {
        ret.status = status::failure;
        return ret;
    }

    if (prop.log_date) {
        std::time_t time = std::time(nullptr);
        std::tm local = *std::localtime(&time);

        char buf[20];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &local);

        ret.date = buf;

        prefix += std::string(buf) + ": ";
    }

    ret.prefix = prefix;
    ret.file = logfile;
    ret.type = type;

    if (prop.output_to_std) {
        if (prop.stream == stream::stderr) {
            std::cerr << prefix << data;
        } else if (prop.stream == stream::stdout) {
            std::cout << prefix << data;
        } else {
            ret.status = status::failure;
            return ret;
        }
    }

    ret.message = data;
    ret.data = prefix + data;

    if (prop.output_to_file && logfile.empty() == false) {
        std::ofstream stream(logfile, std::ios::app);

        if (stream.is_open()) {
            stream << prefix << data;
            stream.close();

            ret.status = status::success;
        } else {
            ret.status = status::failure;
        }
    } else {
        ret.status = status::success;
    }

    return ret;
}
#endif // LIMHAMN_LOGGER_IMPL