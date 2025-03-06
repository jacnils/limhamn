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
    enum class Type {
        Access,
        Error,
        Warning,
        Notice,
        Undefined,
    };

    /**
     * @brief  List of integers representing success or failure.
     */
    enum class Status {
        Success,
        Failure,
        Undefined,
    };

    /**
     * @brief  List of integers representing known output streams
     */
    enum class Stream {
        Stdout,
        Stderr,
        None,
    };

    using LoggerStatus = Status;
    using LoggerErrorType = Type;
    using LoggerStream = Stream;
    using LoggerFile = std::string;
    using LoggerBoolean = bool;
    using LoggerPrefix = std::string;

    /**
     * @brief  Struct containing settings to initialize the logger with.
     */
    struct LoggerProperties {
        LoggerBoolean output_to_std{false};
        LoggerBoolean output_to_file{true};
        LoggerStream stream{Stream::Stderr};
        LoggerBoolean log_date{true};
        LoggerBoolean log_access_to_file{true};
        LoggerBoolean log_error_to_file{true};
        LoggerBoolean log_warning_to_file{true};
        LoggerBoolean log_notice_to_file{true};
        LoggerFile access_log_file{"/var/log/limhamn/access.log"};
        LoggerFile error_log_file{"/var/log/limhamn/error.log"};
        LoggerFile warning_log_file{"/var/log/limhamn/warning.log"};
        LoggerFile notice_log_file{"/var/log/limhamn/notice.log"};
        LoggerPrefix access_log_prefix{"[ACCESS]: "};
        LoggerPrefix error_log_prefix{"[ERROR]: "};
        LoggerPrefix warning_log_prefix{"[WARNING]: "};
        LoggerPrefix notice_log_prefix{"[NOTICE]: "};
    };

    /**
     * @brief  Struct containing the return values of the logger.
     */
    struct LoggerReturn {
        LoggerErrorType type{Type::Undefined};
        LoggerStatus status{Status::Success};
        LoggerStream stream{Stream::None};
        std::string message{};
        std::string date{};
        std::string data{};
        std::string prefix{};
        std::string file{};
    };

    /**
     * @brief  Class that handles logging.
     */
    class Logger {
            LoggerProperties prop{};
        public:
            /**
             * @brief  Constructor for the logger.
             * @param  prop: LoggerProperties struct containing the settings for the logger.
             */
            explicit Logger(const LoggerProperties& prop);
            /**
             * @brief  Default constructor for the logger.
             */
            explicit Logger() = default;
            /**
             * @brief  Default destructor for the logger.
             */
            ~Logger() = default;

            /**
             * @brief  Writes data to the log.
             * @param  type: LoggerErrorType enum representing the type of log.
             * @param  data: std::string containing the data to log.
             * @return LoggerReturn struct containing the return values of the logger.
             */
            [[nodiscard]] LoggerReturn write_to_log(logger::LoggerErrorType type, const std::string& data) const noexcept; // NOLINT
            /**
             * @brief  Overrides the properties of the logger.
             * @param  prop: LoggerProperties struct containing the settings for the logger.
             */
            void override_properties(const LoggerProperties& prop) noexcept;
            /**
             * @brief  Gets the properties of the logger.
             * @return LoggerProperties struct containing the settings for the logger.
             */
            [[nodiscard]] LoggerProperties get() noexcept;
    };
}

#ifdef LIMHAMN_LOGGER_IMPL
inline limhamn::logger::Logger::Logger(const logger::LoggerProperties& prop) {
    this->prop = prop;
}

inline void limhamn::logger::Logger::override_properties(const logger::LoggerProperties& prop) {
    this->prop = prop;
}

inline limhamn::logger::LoggerProperties limhamn::logger::Logger::get() {
    return this->prop;
}

inline limhamn::logger::LoggerReturn limhamn::logger::Logger::write_to_log(const logger::LoggerErrorType type, const std::string& data) const {
    std::string prefix{"[UNKNOWN]: "};
    std::string logfile{"logfile.txt"};
    logger::LoggerReturn ret{};

    if (type == logger::Type::Warning) {
        prefix = prop.warning_log_prefix;
        logfile = prop.warning_log_file;
    } else if (type == logger::Type::Error) {
        prefix = prop.error_log_prefix;
        logfile = prop.error_log_file;
    } else if (type == logger::Type::Access) {
        prefix = prop.access_log_prefix;
        logfile = prop.access_log_file;
    } else if (type == logger::Type::Notice) {
        prefix = prop.notice_log_prefix;
        logfile = prop.notice_log_file;
    } else {
        ret.status = logger::Status::Failure;
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
        if (prop.stream == logger::Stream::Stderr) {
            std::cerr << prefix << data;
        } else if (prop.stream == logger::Stream::Stdout) {
            std::cout << prefix << data;
        } else {
            ret.status = logger::Status::Failure;
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

            ret.status = logger::Status::Success;
        } else {
            ret.status = logger::Status::Failure;
        }
    } else {
        ret.status = logger::Status::Success;
    }

    return ret;
}
#endif // LIMHAMN_LOGGER_IMPL