/* limhamn::socket::uds - Simple, cross platform UDS socket with ASIO.
 * Copyright (c) 2023-2025 Jacob Nilsson
 * Licensed under the MIT license
 *
 * Dependencies: Boost Asio
 * C++ version: >=17
 * File version: 0.1.0 Link: None
 */
#pragma once

#include <string>
#include <functional>
#include <memory>

#define LIMHAMN_SOCKET_UDS

#ifdef LIMHAMN_SOCKET_UDS_IMPL
#include <asio/local/stream_protocol.hpp>
#include <asio.hpp>
#include <iostream>
#include <filesystem>
#endif

namespace limhamn::socket_impl {
    class uds_session;
    class uds_server;
}

/**
 * @brief  Namespace that contains all the socket related classes and functions.
 */
namespace limhamn::socket {
    /**
     * @brief Public class representing a UDS socket
     */
    class uds_server {
        std::string file{};
        std::string read_delimiter{"\n"};
        std::function<std::string(const std::string&)> callback{};
        std::shared_ptr<limhamn::socket_impl::uds_server> server;
#ifdef LIMHAMN_SOCKET_UDS_IMPL
        asio::io_context ctx;
#endif
        bool running{false};
    public:
        uds_server(const std::string& file, const std::function<std::string(const std::string&)>& callback, const std::string& read_delimiter, bool run);
        ~uds_server();

        void run();
        void stop();

        bool is_running();
    };
}

/**
 * @brief  Class representing a UDS socket session
 */
#ifdef LIMHAMN_SOCKET_UDS_IMPL
namespace limhamn::socket_impl {
class uds_session : public std::enable_shared_from_this<uds_session> {
    public:
        explicit uds_session(asio::local::stream_protocol::socket socket,
                const std::function<std::string(const std::string&)>& callback, const std::string& read_delimiter = "\n") : socket_(std::move(socket)), callback_(std::move(callback)), read_delimiter_(std::move(read_delimiter)) {};

        void start() {
            read();
        }
    private:
        void read() {
            auto self = shared_from_this();
            asio::async_read_until(socket_, buffer_, read_delimiter_,
                    [self](asio::error_code ec, std::size_t bytes_transferred) {
                        if (!ec) {
                            std::istream stream(&self->buffer_);
                            std::string data{};
                            std::getline(stream, data);

                            if (!self->socket_.is_open()) {
                                throw std::runtime_error{"read(): socket not open"};
                            }

                            if (!self->callback_) {
                                throw std::runtime_error{"no valid r/w callback"};
                            }

                            const std::string& ret = self->callback_(data);

                            if (!ret.empty()) {
                                self->write(ret);
                            }
                        } else if (ec == asio::error::eof) {
                            return;
                        } else {
                            throw std::runtime_error{"read() failed."};
                        }
                    }
            );
        }
        void write(const std::string& response) {
            auto self = shared_from_this();

            if (!socket_.is_open()) {
                throw std::runtime_error{"socket_ is not open"};
            }

            asio::async_write(socket_, asio::buffer(response),
                    [self](asio::error_code ec, std::size_t) {
                        if (!ec) {
                            self->read();
                            return;
                        }

                        throw std::runtime_error{"async_write() failed: " + ec.message()};
                    }
            );
        }

        asio::local::stream_protocol::socket socket_;
        asio::streambuf buffer_{};
        std::function<std::string(const std::string&)> callback_{};
        std::string read_delimiter_{};
};

class uds_server {
    public:
        uds_server(asio::io_context& ctx, const std::string& path, const std::function<std::string(const std::string&)>& callback, const std::string& read_delimiter) : acceptor_(ctx, asio::local::stream_protocol::endpoint(path)), callback_(std::move(callback)), read_delimiter_(std::move(read_delimiter)) {
                accept();
            }
    private:
        void accept() {
            acceptor_.async_accept(
                    [this](asio::error_code ec, asio::local::stream_protocol::socket socket) {
                        if (!ec) {
                            std::make_shared<uds_session>(std::move(socket), callback_, std::move(read_delimiter_))->start();
                        } else {
                            throw std::runtime_error{"async_accept() failed."};
                        }
                    }
            );
        }

        asio::local::stream_protocol::acceptor acceptor_;
        std::function<std::string(const std::string&)> callback_;
        std::string read_delimiter_;
};
} // namespace limhamn::socket_impl

inline limhamn::socket::uds_server::uds_server(const std::string& file, const std::function<std::string(const std::string&)>& callback,
        const std::string& read_delimiter = "\n", const bool run = false)
    : file(file), callback(callback), read_delimiter(read_delimiter) {
    std::filesystem::remove(this->file);

    this->server = std::make_shared<limhamn::socket_impl::uds_server>(this->ctx, this->file, this->callback, this->read_delimiter);
    if (!server) {
        throw std::runtime_error{"failed to create uds_server object."};
    }

    if (run) {
        this->run();
    }
}

inline bool limhamn::socket::uds_server::is_running() {
    if (!this->server) {
        throw std::runtime_error{"!this->server, cannot get status"};
    }

    return this->running;
}

inline void limhamn::socket::uds_server::run() {
    if (!this->server) {
        throw std::runtime_error{"!this->server, cannot run"};
    }
    if (this->is_running()) {
        throw std::runtime_error{"server already running"};
    }

    this->running = true;
    ctx.run();
}

inline void limhamn::socket::uds_server::stop() {
    if (!this->server) {
        throw std::runtime_error{"already stopped"};
    }
    if (!this->is_running()) {
        throw std::runtime_error{"server already stopped"};
    }

    ctx.stop();
    this->running = false;
}

inline limhamn::socket::uds_server::~uds_server() {
    try {
        this->stop();
    } catch (const std::exception&) {}
}
#endif
