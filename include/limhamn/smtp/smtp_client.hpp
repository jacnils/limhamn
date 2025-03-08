/* limhamn::smtp::client - A simple SMTP client for C++
 * Copyright (c) 2023-2025 Jacob Nilsson
 * Licensed under the MIT license
 *
 * Dependencies: Boost Asio, OpenSSL
 * C++ version: >=17
 * File version: 0.1.0
 * Link: g++ ... -lboost_system -lssl -lcrypto
 */
#pragma once

#include <string>
#include <openssl/evp.h>
#include <boost/system/detail/error_code.hpp>
#include <boost/asio.hpp>

#define LIMHAMN_SMTP_CLIENT

/**
 * @brief  Namespace that contains all the networking related classes and functions.
 */
namespace limhamn::smtp::client {
    /**
     * @brief  Struct that can be used to construct a client.
     */
    struct mail_properties {
        std::string from{};
        std::string to{};
        std::string smtp_server{};
        unsigned int smtp_port{465};
        std::string username{};
        std::string password{};
        std::string subject{};
        std::string data{};
        std::string content_type{};
    };

    /**
     * @brief  Class that represents a client.
     */
    class client {
    public:
        explicit client(const mail_properties& prop);
    };
}

#ifdef LIMHAMN_SMTP_CLIENT_IMPL
namespace _limhamn_smtp_client_impl {
    class smtp_ssl_client {
    public:
        /**
         * @brief Constructor.
         * @param io_context IO context.
         * @param ssl_context SSL context.
         * @param prop Mail properties.
         */
        smtp_ssl_client(boost::asio::io_context& io_context, boost::asio::ssl::context& ssl_context, limhamn::smtp::client::mail_properties prop)
            : resolver_(io_context), socket_(io_context, ssl_context), mail_properties_(std::move(prop)) {}

        /**
         * @brief Start the client.
         */
        void start() {
            auto endpoints = resolver_.resolve(mail_properties_.smtp_server, std::to_string(mail_properties_.smtp_port));
            boost::asio::async_connect(socket_.lowest_layer(), endpoints,
                [this](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&) {
                    if (!ec) {
                        start_tls();
                    } else {
                        throw std::runtime_error("Connect error: " + ec.message());
                    }
                });
        }

    private:
        /**
         * @brief Encode a string to base64.
         * @param input Input string.
         * @return std::string Encoded string.
         */
        static std::string base64_encode(const std::string& input) noexcept {
            BIO* b64 = BIO_new(BIO_f_base64());
            BIO* mem = BIO_new(BIO_s_mem());
            b64 = BIO_push(b64, mem);
            BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
            BIO_write(b64, input.data(), static_cast<int>(input.size()));
            BIO_flush(b64);
            BUF_MEM* buffer_ptr;
            BIO_get_mem_ptr(b64, &buffer_ptr);
            std::string output(buffer_ptr->data, buffer_ptr->length);
            BIO_free_all(b64);
            return output;
        }

        /**
         * @brief Start TLS.
         */
        void start_tls() {
            socket_.lowest_layer().set_option(boost::asio::ip::tcp::no_delay(true));
            socket_.handshake(boost::asio::ssl::stream_base::client);
            send_ehlo();
        }

        /**
         * @brief Send EHLO.
         */
        void send_ehlo() {
            std::string domain = mail_properties_.from.substr(mail_properties_.from.find('@') + 1);
            std::string ehlo_cmd = "EHLO " + domain + "\r\n";
            boost::asio::async_write(socket_, boost::asio::buffer(ehlo_cmd),
                [this](const boost::system::error_code& ec, std::size_t) {
                    if (!ec) {
                        read_response();
                    } else {
                        throw std::runtime_error("EHLO error: " + ec.message());
                    }
                });
        }

        /**
         * @brief Read response.
         */
        void read_response() {
            boost::asio::async_read_until(socket_, response_, "\r\n",
                [this](const boost::system::error_code& ec, std::size_t) {
                    if (!ec) {
                        std::istream response_stream(&response_);
                        std::string response_line;
                        std::getline(response_stream, response_line);
                        if (response_line.substr(0, 3) == "250") {
                            send_auth_login();
                        } else if (response_line.substr(0, 3) == "220") {
                            start_tls();
                        }
                    } else {
                        throw std::runtime_error("Read response error: " + ec.message());
                    }
                });
        }

        /**
         * @brief Send AUTH LOGIN.
         */
        void send_auth_login() {
            std::string auth_cmd = "AUTH LOGIN\r\n";
            boost::asio::async_write(socket_, boost::asio::buffer(auth_cmd),
                [this](const boost::system::error_code& ec, std::size_t) {
                    if (!ec) {
                        send_username();
                    } else {
                        throw std::runtime_error("Auth error: " + ec.message());
                    }
                });
        }

        /**
         * @brief Send username.
         */
        void send_username() {
            std::string encoded_username = base64_encode(mail_properties_.username) + "\r\n";
            boost::asio::async_write(socket_, boost::asio::buffer(encoded_username),
                [this](const boost::system::error_code& ec, std::size_t) {
                    if (!ec) {
                        send_password();
                    } else {
                        throw std::runtime_error("Username error: " + ec.message());
                    }
                });
        }

        /**
         * @brief Send password.
         */
        void send_password() {
            std::string encoded_password = base64_encode(mail_properties_.password) + "\r\n";
            boost::asio::async_write(socket_, boost::asio::buffer(encoded_password),
                [this](const boost::system::error_code& ec, std::size_t) {
                    if (!ec) {
                        send_email_data();
                    } else {
                        throw std::runtime_error("Password error: " + ec.message());
                    }
                });
        }

        /**
         * @brief Assemble email data.
         * @return std::string Email data.
         */
        [[nodiscard]] std::string assemble_email_data() const {
            std::string email_data = "MAIL FROM:" + mail_properties_.from + "\r\n"
                                     "RCPT TO:" + mail_properties_.to + "\r\n"
                                     "DATA\r\n"
                                     "Subject:" + mail_properties_.subject + "\r\n";

            if (mail_properties_.content_type.empty()) {
                email_data += "Content-Type: text/plain; charset=\"utf-8\"\r\n";
            } else {
                email_data += "Content-Type: " + mail_properties_.content_type + "\r\n";
            }

            email_data += mail_properties_.data + "\r\n";
            email_data += ".\r\n";

            return email_data;
        }

        /**
         * @brief Send email data.
         */
        void send_email_data() {
            boost::asio::async_write(socket_, boost::asio::buffer(assemble_email_data()),
                [this](const boost::system::error_code& ec, std::size_t) {
                    if (!ec) {
                        read_response();
                    } else {
                        throw std::runtime_error("Email data error: " + ec.message());
                    }
                });
        }

        boost::asio::ip::tcp::resolver resolver_;
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
        boost::asio::streambuf response_;
        limhamn::smtp::client::mail_properties mail_properties_;
    };
}

inline limhamn::smtp::client::client::client(const mail_properties& prop) {
    boost::asio::io_context ctx;
    boost::asio::ssl::context ssl_ctx(boost::asio::ssl::context::tlsv13);
    ssl_ctx.set_default_verify_paths();
    _limhamn_smtp_client_impl::smtp_ssl_client client(ctx, ssl_ctx, prop);
    client.start();
    ctx.run();
}
#endif // LIMHAMN_SMTP_CLIENT_IMPL