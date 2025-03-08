//
// Created by Jacob Nilsson on 2025-03-07.
//

#include <iostream>

#define LIMHAMN_ARGUMENT_MANAGER_IMPL
#define LIMHAMN_DATABASE_SQLITE3
#define LIMHAMN_DATABASE_POSTGRESQL
#define LIMHAMN_DATABASE_ICONV
#define LIMHAMN_DATABASE_IMPL
#define LIMHAMN_HTTP_CLIENT_IMPL
#define LIMHAMN_HTTP_SERVER_IMPL
#define LIMHAMN_HTTP_UTILS_IMPL
#define LIMHAMN_INI_PARSER_IMPL
#define LIMHAMN_LOGGER_IMPL
#define LIMHAMN_SMTP_CLIENT_IMPL

#include <limhamn/argument_manager/argument_manager.hpp>
#include <limhamn/database/database.hpp>
#include <limhamn/http/http_server.hpp>
#include <limhamn/http/http_client.hpp>
#include <limhamn/http/http_utils.hpp>
#include <limhamn/ini/ini_parser.hpp>
#include <limhamn/logger/logger.hpp>
#include <limhamn/smtp/smtp_client.hpp>

#include "macros.hpp"

int main() {
    REQUIRE(1==1); // just to test the REQUIRE macro
}