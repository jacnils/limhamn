/* limhamn::http::server - A simple HTTP server for C++
 * Copyright (c) 2023-2025 Jacob Nilsson
 * Licensed under the MIT license
 *
 * Dependencies: Boost Beast, Boost Asio, OpenSSL
 * C++ version: >=17
 * File version: 0.1.0
 * Link: g++ ... -lboost_system -lssl -lcrypto
 */
#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#ifdef LIMHAMN_HTTP_SERVER_IMPL
#include <filesystem>
#include <sstream>
#include <fstream>
#include <random>
#include <algorithm>
#include <boost/system/detail/error_code.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#endif

#define LIMHAMN_HTTP_SERVER

/**
 * @brief  Namespace that contains all the networking related classes and functions.
 */
namespace limhamn::http::server {
    /**
     * @brief  Struct that represents a cookie.
     */
    struct cookie {
        std::string name{};
        std::string value{};
        int64_t expires{};
        std::string path{"/"};
        std::string domain{};
        std::string same_site{"Strict"};
        std::vector<std::string> attributes{};
        bool http_only{false};
        bool secure{false};
        std::unordered_map<std::string, std::string> extra_attributes{};
    };

    /**
     * @brief  Struct that represents an HTTP header.
     */
    struct header {
        std::string name{};
        std::string data{};
    };

    /**
     * @brief  An enum class that represents the type of redirect.
     */
    enum class redirect_type {
        permanent,
        temporary,
    };

    /**
     * @brief  Struct that contains the server settings.
     */
    struct server_settings {
        int port{8080};
        bool enable_session{true};
        std::string session_directory{"./"};
        std::string session_cookie_name{"session_id"};
        std::vector<std::string> associated_session_cookies{};
        int64_t max_request_size{1024 * 1024 * 1024};
        std::vector<std::pair<std::string, int>> rate_limits{};
        std::vector<std::string> blacklisted_ips{};
        std::vector<std::string> whitelisted_ips{"127.0.0.1"};
        int default_rate_limit{100};
        bool trust_x_forwarded_for{false};
        bool session_is_secure{false};
    };

    /**
     * @brief  Struct that contains the request data.
     */
    struct request {
        std::string endpoint{};
        std::unordered_map<std::string, std::string> query{};
        std::string content_type{};
        std::string body{};
        std::string raw_body{};
        std::string method{};
        std::string ip_address{};
        std::string user_agent{};
        unsigned int version{};
        std::vector<cookie> cookies{};
        std::unordered_map<std::string, std::string> session{};
        std::string session_id{};
        std::unordered_map<std::string, std::string> fields{};
    };

    /**
     * @brief  Struct that contains the response data.
     */
    struct response {
        int http_status{200};
        std::string body{};
        std::string content_type{"application/json"};
        std::string allow_origin{"*"};
        bool stop{false};
        std::vector<cookie> cookies{};
        std::vector<std::string> delete_cookies{};
        std::unordered_map<std::string, std::string> session{};
        std::string location{};
        redirect_type redirect_status{redirect_type::temporary};
        std::vector<header> headers{};
    };

    /**
     * @brief  Class that represents a server.
     */
    class server {
    public:
        /**
         * @brief  Constructor for the server class
         * @param  settings The settings for the server
         * @param  callback The function to call when a request is made
         */
        server(const server_settings& settings, const std::function<response(const request&)>& callback);
        /**
         * @brief  Start the server
         */
        static void stop();
    };
}

#ifdef LIMHAMN_HTTP_SERVER_IMPL
namespace _limhamn_http_server_impl {
    inline std::function<limhamn::http::server::response(const limhamn::http::server::request&)> generate_response_from_endpoint;
    void stop();
    static bool enable_session{true};
    static std::string session_dir{"./sessions"};
    static std::string session_cookie_name{"session_id"};
    static std::vector<std::string> associated_session_cookies{};
    static int64_t max_request_size{1024 * 1024 * 1024};
    static int default_rate_limit{100};
    static std::vector<std::pair<std::string, int>> rate_limited_endpoints{};
    static std::vector<std::string> blacklisted_ips{};
    static std::vector<std::string> whitelisted_ips{};
    using RateLimitTracker = std::unordered_map<std::string, std::tuple<std::string, int64_t, int64_t>>;
    static RateLimitTracker rate_limit_tracker;
    static bool trust_x_forwarded_for{false};
    static bool session_is_secure{false};

    inline std::string convert_unix_millis_to_gmt(const int64_t unix_millis) {
        if (unix_millis == -1) {
            return "Thu, 01 Jan 1970 00:00:00 GMT";
        }

        std::time_t time = unix_millis / 1000;
        std::tm* tm = std::gmtime(&time);
        char buffer[80];
        std::strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT", tm);

        return {(buffer)};
    }

    /**
     * @brief A class that represents a session
     */
    class session : public std::enable_shared_from_this<session> {
        public:
            explicit session(boost::asio::ip::tcp::socket socket) : net_socket(std::move(socket)) {}

            /**
             * @brief Starts the session
             */
            void start() {
                read_request();
            }

            /**
             * @brief Stops the session
             */
            void stop() {
                boost::beast::error_code ec;
                static_cast<void>(net_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec));
                static_cast<void>(net_socket.close(ec));
            }
        private:
            boost::asio::ip::tcp::socket net_socket;
            boost::beast::flat_buffer net_buffer;
            boost::beast::http::request<boost::beast::http::string_body> net_request;
            boost::beast::http::response<boost::beast::http::string_body> net_response;
            std::shared_ptr<boost::beast::http::request_parser<boost::beast::http::string_body>> parser;

            std::string get_ip() const {
                if (trust_x_forwarded_for) {
                    auto it = net_request.find("X-Forwarded-For");
                    if (it != net_request.end()) {
                        std::string x_forwarded_for = it->value();
                        std::size_t pos = x_forwarded_for.find(',');
                        if (pos != std::string::npos) {
                            return x_forwarded_for.substr(0, pos);
                        }
                        return x_forwarded_for;
                    }
                }
                return net_socket.remote_endpoint().address().to_string();
            }

            static std::unordered_map<std::string, std::string> parse_fields(const std::string& _body) {
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

            static std::unordered_map<std::string, std::string> parse_query_string(const std::string& url) {
                std::size_t pos = url.find('?');

                if (pos == std::string::npos) {
                    return {};
                }

                std::string req = "&" + url.substr(pos + 1);

                return parse_fields(req);
            }

            static std::string generate_random_string(const int length = 64) {
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

            void read_request() {
                auto self = shared_from_this();
                parser = std::make_shared<boost::beast::http::request_parser<boost::beast::http::string_body>>();

                if (max_request_size != -1) {
                    parser->body_limit(max_request_size);
                } else {
                    parser->body_limit((std::numeric_limits<std::uint64_t>::max)());
                }

                const auto ip = net_socket.remote_endpoint().address().to_string();

                for (const auto& it : whitelisted_ips) {
                    if (it != ip) {
                        continue;
                    }

                    boost::beast::http::async_read(
                        net_socket,
                        net_buffer,
                        *parser,

                        [self](const boost::beast::error_code& ec, std::size_t transferred_bytes) {
                            self->on_read(ec, transferred_bytes);
                        }
                    );

                    return;
                }

                const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                const auto endpoint = std::string(net_request.target().data(), net_request.target().size());
                const std::string key = ip + ":" + endpoint;

                for (const auto& it : blacklisted_ips) {
                    if (ip == it) {
                        return;
                    }
                }

                int rate_limit = default_rate_limit;
                for (const auto& [ep, limit] : rate_limited_endpoints) {
                    if (ep == endpoint) {
                        rate_limit = limit;
                        break;
                    }
                }

                if (rate_limit_tracker.find(key) == rate_limit_tracker.end()) {
                    rate_limit_tracker[key] = {endpoint, now, 1};
                } else {
                    auto& [ep, last_request_time, request_count] = rate_limit_tracker[key];
                    if (now - last_request_time < 60000) {
                        if (request_count >= rate_limit) {
                            return;
                        }
                        request_count++;
                    } else {
                        last_request_time = now;
                        request_count = 1;
                    }
                }

                boost::beast::http::async_read(
                    net_socket,
                    net_buffer,
                    *parser,

                    [self](const boost::beast::error_code& ec, std::size_t transferred_bytes) {
                        self->on_read(ec, transferred_bytes);
                    }
                );
            }

            /**
             * @brief Handles the read request
             * @param ec The error code
             * @param transferred_bytes The amount of bytes transferred
             */
            void on_read(const boost::beast::error_code& ec, std::size_t transferred_bytes) {
                static_cast<void>(transferred_bytes); // prevents an unused parameter warning

                if (!ec) {
                    net_request = parser->release();
                    handle_request();
                }
            }

            static std::vector<limhamn::http::server::cookie> get_cookies_from_request(const std::string& cookie_header) {
                std::vector<limhamn::http::server::cookie> cookies;
                std::string cookie_str = cookie_header + ";";

                while (cookie_str.find(';') != std::string::npos) {
                    std::string cookie = cookie_str.substr(0, cookie_str.find(';'));
                    cookie_str = cookie_str.substr(cookie_str.find(';') + 1);

                    std::string name = cookie.substr(0, cookie.find('='));
                    std::string value = cookie.substr(cookie.find('=') + 1);

                    if (!name.empty() && !value.empty()) {
                        if (name.front() == ' ') {
                            name = name.substr(1);
                        }
                        cookies.push_back({name, value});
                    }
                }

                return cookies;
            }

            static std::unordered_map<std::string, std::string> read_from_session_file(const std::string& f) {
                std::unordered_map<std::string, std::string> session;

                std::ifstream file(f);

                if (!file.good()) {
                    file.close();
                    return {};
                }

                if (!file.is_open()) {
                    throw std::runtime_error("failed to open session file (read_from_session_file()): " + f);
                }

                std::string line{};
                while (std::getline(file, line)) {
                    if (line.find('=') != std::string::npos) {
                        std::string key = line.substr(0, line.find('='));
                        std::string value = line.substr(line.find('=') + 1);

                        session[key] = value;
                    }
                }

                file.close();

                return session;
            }

            static void write_to_session_file(const std::string& f, const std::unordered_map<std::string, std::string>& session) {
                std::ofstream file(f, std::ios::trunc);

                if (!file.is_open() || !file.good()) {
                    throw std::runtime_error("failed to open session file (write_to_session_file()): " + f);
                }

                for (const auto& it : session) {
                    file << it.first << "=" << it.second << "\n";
                }

                file.close();
            }

            /**
             * @brief Handles the request
             */
            void handle_request() {
                if (net_request.method() == boost::beast::http::verb::options) {
                    net_response.result(boost::beast::http::status::no_content);
                    net_response.set(boost::beast::http::field::allow, "GET, HEAD, OPTIONS");
                    net_response.set(boost::beast::http::field::access_control_allow_origin, "*");
                    net_response.set(boost::beast::http::field::access_control_allow_headers, "Content-Type");
                } else {
                    net_response = boost::beast::http::response<boost::beast::http::string_body>();
                    limhamn::http::server::request request{};

                    request.endpoint = std::string(net_request.target().data(), net_request.target().size());

                    std::ostringstream oss;
                    oss << net_request;
                    request.raw_body = oss.str(); // TODO: a little inefficient, but works for now
                    request.body = net_request.body();
                    request.fields = parse_fields(request.body);
                    request.ip_address = get_ip();
                    request.method = net_request.method_string();
                    request.version = net_request.version();
                    request.user_agent = net_request.at(boost::beast::http::field::user_agent).data();

                    if (request.endpoint.find('?') != std::string::npos) {
                        request.query = parse_query_string(request.endpoint);
                        request.endpoint = request.endpoint.substr(0, request.endpoint.find('?'));
                    }

                    if (net_request.find("Cookie") != net_request.end()) {
                        request.cookies = get_cookies_from_request(net_request.find("Cookie")->value());
                    }

                    std::string session_id{};
                    bool session_id_found = false;
                    for (const auto& it : request.cookies) {
                        if (it.name == session_cookie_name && !it.value.empty() && enable_session) {
                            session_id = it.value;
                            session_id_found = true;
                            break;
                        }
                    }

                    bool erase_associated = false;
                    if (session_id_found) {
                        session_id.erase(std::remove(session_id.begin(), session_id.end(), '/'), session_id.end());
                        std::filesystem::path session_file = session_dir + "/session_" + session_id + ".txt";
                        if (!std::filesystem::exists(session_file)) {
                            erase_associated = true;
                            // remove associated session cookies and session cookie from request
                            for (const auto& it : associated_session_cookies) {
                                request.cookies.erase(
                                    std::remove_if(request.cookies.begin(), request.cookies.end(),
                                                   [&it](const limhamn::http::server::cookie& cookie) {
                                                       return cookie.name == it;
                                                   }),
                                    request.cookies.end()
                                );
                            }
                            request.cookies.erase(
                                std::remove_if(request.cookies.begin(), request.cookies.end(),
                                               [](const limhamn::http::server::cookie& cookie) {
                                                   return cookie.name == session_cookie_name;
                                               }),
                                request.cookies.end()
                            );

                            request.session.clear();
                            request.session_id.clear();
                        } else {
                            request.session = read_from_session_file(session_file);
                            request.session_id = session_id;
                        }
                    }

                    limhamn::http::server::response response = generate_response_from_endpoint(request);

                    if (!session_id_found && enable_session) {
                        session_id = generate_random_string();

                        for (const auto& it : response.cookies) {
                            if (it.name == session_cookie_name) {
                                session_id_found = true;
                                break;
                            }
                        }

                        if (!session_id_found) {
                            response.cookies.push_back({session_cookie_name, session_id, 0, "/", .same_site = "Strict", .http_only = true, .secure = session_is_secure});
                        }
                    } else if (enable_session) {
                        std::string session_file = session_dir + "/session_" + session_id + ".txt";
                        std::unordered_map<std::string, std::string> stored = read_from_session_file(session_file);

                        for (const auto& it : response.session) {
                            stored[it.first] = it.second;
                        }

                        write_to_session_file(session_file, stored);
                    }

                    for (const auto& it : response.cookies) {
                        std::string cookie_str = it.name + "=" + it.value + "; ";
                        if (it.expires != 0) {
                            cookie_str += "Expires=" + convert_unix_millis_to_gmt(it.expires) + "; ";
                        } else {
                            cookie_str += "Expires=session; ";
                        }
                        if (it.http_only) {
                            cookie_str += "HttpOnly; ";
                        }
                        if (it.secure) {
                            cookie_str += "Secure; ";
                        }
                        if (!it.path.empty()) {
                            cookie_str += "Path=" + it.path + "; ";
                        }
                        if (!it.domain.empty()) {
                            cookie_str += "Domain=" + it.domain + "; ";
                        }
                        if (!it.same_site.empty() && (it.same_site == "Strict" || it.same_site == "Lax" || it.same_site == "None")) {
                            cookie_str += "SameSite=" + it.same_site + "; ";
                        }
                        for (const auto& attribute : it.attributes) {
                            cookie_str += attribute + "; ";
                        }
                        for (const auto& attribute : it.extra_attributes) {
                            cookie_str += attribute.first + "=" + attribute.second + "; ";
                        }

                        net_response.insert(boost::beast::http::field::set_cookie, cookie_str);
                    }

                    if (erase_associated) {
                        for (const auto& it : associated_session_cookies) {
                            response.delete_cookies.push_back(it);
                        }
                    }

                    for (const auto& it : response.delete_cookies) {
                        std::string cookie_str = it + "=; Expires=Thu, 01 Jan 1970 00:00:00 GMT; Max-Age=0; Path=/; ";
                        net_response.insert(boost::beast::http::field::set_cookie, cookie_str);
                    }

                    if (response.stop) {
                        stop();
                        _limhamn_http_server_impl::stop();
                        return;
                    }

                    if (!response.location.empty()) {
                        if (response.redirect_status == limhamn::http::server::redirect_type::temporary) {
                            net_response.result(boost::beast::http::status::temporary_redirect);
                        } else if (response.redirect_status == limhamn::http::server::redirect_type::permanent) {
                            net_response.result(boost::beast::http::status::moved_permanently);
                        }
                        net_response.set(boost::beast::http::field::location, response.location);
                    } else {
                        net_response.result(response.http_status);
                    }

                    for (const auto& it : response.headers) {
                        net_response.set(it.name, it.data);
                    }

                    net_response.set(boost::beast::http::field::content_type, response.content_type);
                    net_response.set(boost::beast::http::field::access_control_allow_origin, response.allow_origin);
                    net_response.body() = response.body;
                }

                const auto self = shared_from_this();

                boost::beast::http::async_write(
                    net_socket,
                    net_response,
                    [self](const boost::beast::error_code& ec, std::size_t transferred_bytes) {
                        static_cast<void>(transferred_bytes);
                        self->on_write(ec);
                    }
                );
            }

            /**
             * @brief Handles the write request
             * @param ec The error code
             */
            void on_write(const boost::beast::error_code& ec) {
                if (!ec) {
                    boost::beast::error_code close_ec;
                    static_cast<void>(net_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, close_ec));
                }
            }
    };

    /**
     * @brief A class that represents a listener
     */
    class listener {
        public:
            boost::asio::io_context ioc;
            boost::asio::io_context& ref_ioc;
            boost::asio::ip::tcp::acceptor acceptor;

            static listener& get_instance() {
                static listener instance;
                return instance;
            }

            /**
             * @brief Constructs a new listener object
             * @param port The port to listen on
             */
            explicit listener(const int port = 8080) : ref_ioc(ioc), acceptor(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
                run();
                ioc.run();
            }

            /**
             * @brief Destroys the listener object
             */
            ~listener() {
                stop();
            }

            void stop() {
                acceptor.cancel();
                ioc.stop();
            }

            /**
             * @brief Runs the listener
             */
            void run() {
                acceptor.async_accept(
                    [this](const boost::beast::error_code& ec, boost::asio::ip::tcp::socket socket) {
                        if (!ec) {
                            std::make_shared<session>(std::move(socket))->start();
                        }

                        this->run();
                    }
                );
            }
    };

    inline void stop() {
        listener::get_instance().stop();
    }
}

inline limhamn::http::server::server::server(const server_settings& settings, const std::function<limhamn::http::server::response(const limhamn::http::server::request&)>& callback) {
    _limhamn_http_server_impl::generate_response_from_endpoint = callback;
    _limhamn_http_server_impl::session_dir = settings.session_directory;
    _limhamn_http_server_impl::enable_session = settings.enable_session;
    _limhamn_http_server_impl::session_cookie_name = settings.session_cookie_name;
    _limhamn_http_server_impl::associated_session_cookies = settings.associated_session_cookies;
    _limhamn_http_server_impl::max_request_size = settings.max_request_size;
    _limhamn_http_server_impl::rate_limited_endpoints = settings.rate_limits;
    _limhamn_http_server_impl::default_rate_limit = settings.default_rate_limit;
    _limhamn_http_server_impl::blacklisted_ips = settings.blacklisted_ips;
    _limhamn_http_server_impl::whitelisted_ips = settings.whitelisted_ips;
    _limhamn_http_server_impl::trust_x_forwarded_for = settings.trust_x_forwarded_for;
    _limhamn_http_server_impl::listener listener{settings.port};
}

inline void limhamn::http::server::server::stop() {
    _limhamn_http_server_impl::stop();
}
#endif // LIMHAMN_HTTP_SERVER_IMPL
