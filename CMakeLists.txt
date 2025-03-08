cmake_minimum_required(VERSION 3.1...3.29)

project(limhamn_test VERSION 0.1.0
    DESCRIPTION "A test project for limhamn."
    HOMEPAGE_URL "https://github.com/jacnils/limhamn"
)

add_compile_definitions(LIMHAMN_VERSION="${PROJECT_VERSION}")

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(PostgreSQL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Boost REQUIRED CONFIG COMPONENTS system)
find_package(OpenSSL REQUIRED)

include_directories(include)

set(CMAKE_CXX_STANDARD 17)
set(PROJECT_SOURCE_FILES
        tests/main.cpp
)

add_executable(limhamn_test ${PROJECT_SOURCE_FILES})

target_link_libraries(limhamn_test
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
        PostgreSQL::PostgreSQL
        SQLite::SQLite3
)

enable_testing()
add_test(NAME limhamn_test COMMAND limhamn_test)