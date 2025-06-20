# limhamn

Collection of tiny, single-header wrappers for C++

## Why

Many of my projects are written in C++, and as often happens using C++,
I find myself using third-party libraries. Often times, these third-party libraries
are still a bit too verbose and low-level for my taste, so I end up writing wrappers.
But then these wrappers become boilerplate. So I decided to make a collection of
wrappers for common tasks that I can use in my projects.

Some of them require external dependencies, some of them require different C++ versions,
some of them are perhaps a little bit ugly. But they all serve a purpose for me, and I
hope they can serve a purpose for you too.

I've chosen to make them single header, so that any project can simply include
limhamn as a submodule and include the headers they need. This way, you can
easily update limhamn and get the latest features and bug fixes.

## Dependencies

The dependencies will depend on which headers you include. Some of them have no
dependencies, others will require including the header and linking against
some external libraries.

## Installation

To install limhamn, simply add it as a submodule to your project:

```sh
git submodule init
git submodule add https://github.com/jacnils/limhamn libs/limhamn
```

Then, include the headers you need in your project.

## List of headers

- `limhamn/logger/logger.hpp`: Simple logger for C++ projects.
  - Dependencies: None
  - Usage: `#include "limhamn/logger/logger.hpp"`
  - Prerequisites: `#define LIMHAMN_LOGGER_IMPL` (for implementation)
  - C++ version: C++17(?)
  - File version: 0.1.0
- `limhamn/argument_manager/argument_manager.hpp`: Simple argument manager for C++ projects.
  - Dependencies: None
  - Usage: `#include "limhamn/argument_manager/argument_manager.hpp"`
  - Prerequisites: `#define LIMHAMN_ARGUMENT_MANAGER_IMPL` (for implementation)
  - C++ version: C++17(?)
  - File version: 0.1.0
- `limhamn/ini/ini_parser.hpp`: Simple INI parser for C++ projects.
  - Dependencies: None
  - Usage: `#include "limhamn/ini_parser/ini_parser.hpp"`
  - Prerequisites: `#define LIMHAMN_INI_PARSER_IMPL` (for implementation)
  - C++ version: C++17(?)
  - File version: 0.1.0
- `limhamn/database/database.hpp`: Simple database manager for C++ projects.
  - Dependencies: SQLite3 and/or PostgreSQL and iconv (optional but strongly recommended)
  - Usage: `#include "limhamn/database/database.hpp"`
  - Prerequisites:
    - `#define LIMHAMN_DATABASE_IMPL` (for implementation)
    - `#define LIMHAMN_DATABASE_SQLITE3` (for SQLite3 support)
    - `#define LIMHAMN_DATABASE_POSTGRESQL` (for PostgreSQL support)
    - `#define LIMHAMN_DATABASE_ICONV` (for iconv support)
  - C++ version: C++17(?)
  - File version: 0.1.0
- `limhamn/http/http_client.hpp`: Simple HTTP client for C++ projects.
  - Dependencies: Boost.Beast, Boost.Asio, Boost.System, OpenSSL
  - Usage: `#include "limhamn/http/http_client.hpp"`
  - Prerequisites: `#define LIMHAMN_HTTP_CLIENT_IMPL` (for implementation)
  - Note: Blocking; use `std::thread` if necessary.
  - Note: Synchronous; concurrent.
  - C++ version: C++17(?)
  - File version: 0.1.0
- `limhamn/http/http_server.hpp`: Simple HTTP server for C++ projects.
  - Dependencies: Boost.Beast, Boost.Asio, Boost.System, OpenSSL
  - Usage: `#include "limhamn/http/http_server.hpp"`
  - Prerequisites: `#define LIMHAMN_HTTP_SERVER_IMPL` (for implementation)
  - Note: Asynchronous
  - Note: Blocking; use `std::thread` if necessary.
  - C++ version: C++17(?)
  - File version: 0.1.0
- `limhamn/http/http_utils.hpp`: Simple HTTP utilities for C++ projects.
  - Dependencies: OpenSSL
  - Usage: `#include "limhamn/http/http_utils.hpp"`
  - Prerequisites: `#define LIMHAMN_HTTP_UTILS_IMPL` (for implementation)
  - C++ version: C++17(?)
  - File version: 0.1.0
- `limhamn/smtp/smtp_client.hpp`: Simple STARTTLS SMTP client for C++ projects.
  - Dependencies: Boost.Asio, Boost.System, OpenSSL
  - Usage: `#include "limhamn/smtp/smtp_client.hpp"`
  - Prerequisites: `#define LIMHAMN_SMTP_CLIENT_IMPL` (for implementation)
  - Note: Blocking; use `std::thread` if necessary.
  - C++ version: C++17(?)
  - File version: 0.1.0
  - Note: Does not receive emails (as of now), its purpose is to send emails for e.g. registration.
  - Note: Currently doesn't provide good feedback or really any kind of response. It throws exceptions on failure, though.
- `limhamn/primitive/primitive.hpp`: Primitive drawing for C++ (Xlib and canvas support)
  - Dependencies: Cairo, Pango, XLib (optional)
  - Usage: `#include "limhamn/primitive/primitive.hpp"`
  - Prerequisites:
    - `#define LIMHAMN_PRIMITIVE_IMPL` (for implementation)
    - `#define LIMHAMN_PRIMITIVE_X11` (for Xlib support)
    - `#define LIMHAMN_PRIMITIVE_CANVAS` (for canvas support)
  - C++ version: C++20
  - File version: 0.1.0
  - Note: Quite basic, but functional. Does not include any window/client management.

## Naming

Limhamn is a place in Malmö, Sweden. I thought it would be fun to name my projects after
random places in Sweden. I have no real connection to Limhamn, but I thought it was a nice
name (meaning "glue harbor" in Swedish). I study in Malmö though so close enough.
