/* limhamn::database - Thin wrapper around SQLite3 and PostgreSQL
 * Copyright (c) 2023-2025 Jacob Nilsson
 * Licensed under the MIT license
 *
 * Dependencies: iconv (optional), SQLite3, PostgreSQL
 * - iconv: Opt-out, #define LIMHAMN_DATABASE_ICONV 0
 * - SQLite3: Opt-out, #define LIMHAMN_DATABASE_SQLITE3 0
 * - PostgreSQL: Opt-in, #define LIMHAMN_DATABASE_POSTGRESQL 1
 * C++ version: >=17
 * File version: 0.1.0
 * Link: g++ ... -lsqlite3 -lpq -liconv -DLIMHAMN_DATABASE_ICONV -DLIMHAMN_DATABASE_POSTGRESQL -DLIMHAMN_DATABASE_SQLITE3
 */

#pragma once

#ifdef LIMHAMN_DATABASE_IMPL
#include <iostream>
#include <fstream>
#include <stdexcept>
#endif
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

#ifndef LIMHAMN_DATABASE_SQLITE3
#ifndef LIMHAMN_DATABASE_POSTGRESQL
#define LIMHAMN_DATABASE_SQLITE3
#endif
#endif
#ifndef LIMHAMN_DATABASE_ICONV
#define LIMHAMN_DATABASE_ICONV
#endif

#ifdef LIMHAMN_DATABASE_SQLITE3
#include <sqlite3.h>
#endif
#ifdef LIMHAMN_DATABASE_POSTGRESQL
#include <libpq-fe.h>
#endif
#ifdef LIMHAMN_DATABASE_ICONV
#include <iconv.h>
#endif

/**
 * @brief Namespace for database related functions and classes.
 */
namespace limhamn::database {
   /**
    * @brief Remove non-UTF-8 characters from a string.
    * @param input Input string.
    * @return std::string Cleaned string.
    */
    std::string remove_non_utf8(const std::string& input);

#ifdef LIMHAMN_DATABASE_SQLITE3
    /**
     * @brief Class for database operations with SQLite3.
     */
    class sqlite3_database {
            sqlite3* sqlite3_db{};
            std::string database{};
            bool is_good{false};

            /**
             * @brief Bind parameters to a prepared statement.
             *
             * @param stmt Statement to bind to.
             * @param index Index of the parameter.
             * @param value Value to bind.
             * @param args Remaining parameters.
             */
            template<typename T, typename... Args>
            void bind_parameters(sqlite3_stmt* stmt, int index, T value, Args... args);

            /**
             * @brief Temporary storage for data. Do not use this directly.
             */
            static std::vector<std::unordered_map<std::string, std::string>> tmp;
            /**
             * @brief Callback function for sqlite3_exec.
             *
             * @param data Pointer to data.
             * @param argc Number of columns.
             * @param argv Column values.
             * @param name Column names.
             * @return int 0.
             */
            static int callback(void* data, int argc, char** argv, char** name);

            /**
             * @brief Bind parameters to a prepared statement.
             *
             * @param stmt Statement to bind to.
             * @param index Index of the parameter.
             */
            static void bind_parameters(sqlite3_stmt* stmt, int index);
            /**
             * @brief Bind a parameter to a prepared statement.
             *
             * @param stmt Statement to bind to.
             * @param index Index of the parameter.
             * @param value Value to bind.
             */
            static void bind_parameter(sqlite3_stmt* stmt, int index, int value);
            /**
             * @brief Bind a parameter to a prepared statement.
             *
             * @param stmt Statement to bind to.
             * @param index Index of the parameter.
             * @param value Value to bind.
             */
            static void bind_parameter(sqlite3_stmt* stmt, int index, int64_t value);
            /**
             * @brief Bind a parameter to a prepared statement.
             *
             * @param stmt Statement to bind to.
             * @param index Index of the parameter.
             * @param value Value to bind.
             */
            static void bind_parameter(sqlite3_stmt* stmt, int index, double value);
            /**
             * @brief Bind a parameter to a prepared statement.
             *
             * @param stmt Statement to bind to.
             * @param index Index of the parameter.
             * @param value Value to bind.
             */
            static void bind_parameter(sqlite3_stmt* stmt, int index, const std::string& value);
            /**
             * @brief Bind a parameter to a prepared statement.
             *
             * @param stmt Statement to bind to.
             * @param index Index of the parameter.
             * @param value Value to bind.
             */
            static void bind_parameter(sqlite3_stmt* stmt, int index, const char* value);
        public:
            /**
             * @brief Query the database, returning data.
             * @param query Query to execute.
             * @param args Arguments.
             * @return std::vector<std::unordered_map<std::string, std::string>> Data.
             */
            template <typename... Args>
            bool exec(const std::string& query, Args... args);
            /**
             * @brief Query the database, returning data.
             * @param query Query to execute.
             * @param args Arguments.
             * @return std::vector<std::unordered_map<std::string, std::string>> Data.
             */
            template <typename... Args>
            std::vector<std::unordered_map<std::string, std::string>> query(const std::string& query, Args... args);
            /**
             * @brief Query the database, returning data.
             * @param query Query to execute.
             * @return std::vector<std::unordered_map<std::string, std::string>> Data.
             */
            [[nodiscard]] std::vector<std::unordered_map<std::string, std::string>> query(const std::string& query) const;
            /**
             * @brief Execute an SQL command.
             * @param query Query to execute.
             * @return bool True if successful.
             */
            [[nodiscard]] bool exec(const std::string& query) const;
            /**
             * @brief Check if the database is good.
             * @return bool True if good.
             */
            [[nodiscard]] bool good() const;
            /**
             * @brief Check if the database is open.
             * @return bool True if open.
             */
            [[nodiscard]] bool is_open() const;
            /**
             * @brief Open a database from file.
             * @param database Database to open.
             */
            void open(const std::string& database);
            /**
             * @brief Close the open database.
             */
            void close();
            /**
             * @brief Check if the database is empty.
             * @return bool
             */
            [[nodiscard]] bool empty() const;
            /**
             * @brief Validate an SQL statement.
             * @param query Query to validate.
             * @return bool True if valid.
             */
            [[nodiscard]] bool validate(const std::string& query) const;
            /**
             * @brief Get the last insertion.
             * @return std::int64_t Last insertion.
             */
            [[nodiscard]] std::int64_t get_last_insertion() const;
            /**
             * @brief Constructor.
             */
            sqlite3_database();
            /**
             * @brief Constructor.
             * @param database Database to open.
             */
            explicit sqlite3_database(const std::string& database);
            /**
             * @brief Destructor.
             */
            ~sqlite3_database();
    };
#endif

#ifdef LIMHAMN_DATABASE_POSTGRESQL
    /**
     * @brief Class for database operations with PostgreSQL.
     */
    class postgresql_database {
            PGconn* pg_conn{};
            std::string host{};
            std::string user{};
            std::string password{};
            std::string database{};
            bool is_good{false};
            int port{5432};

            /**
             * @brief Convert a value to a string.
             * @param value Value to convert.
             * @return std::string String.
             */
            template <typename T>
            std::string to_string(const T& value);
        public:
            /**
             * @brief Query the database, returning data.
             * @param query Query to execute.
             * @param args Arguments.
             * @return std::vector<std::unordered_map<std::string, std::string>> Data.
             */
            template <typename... Args>
            bool exec(const std::string& query, Args... args);

            /**
             * @brief Query the database, returning data.
             * @param query Query to execute.
             * @param args Arguments.
             * @return std::vector<std::unordered_map<std::string, std::string>> Data.
             */
            template <typename... Args>
            std::vector<std::unordered_map<std::string, std::string>> query(const std::string& query, Args... args);
            /**
             * @brief Query the database, returning data.
             * @param query Query to execute.
             * @return std::vector<std::unordered_map<std::string, std::string>> Data.
             */
            [[nodiscard]] std::vector<std::unordered_map<std::string, std::string>> query(const std::string& query) const;
            /**
             * @brief Execute an SQL command.
             * @param query Query to execute.
             * @return bool True if successful.
             */
            [[nodiscard]] bool exec(const std::string& query) const;
            /**
             * @brief Check if the database is good.
             * @return bool True if good.
             */
            [[nodiscard]] bool good() const;
            /**
             * @brief Check if the database is open.
             * @return bool True if open.
             */
            [[nodiscard]] bool is_open() const;
            /**
             * @brief Open a database from file.
             * @param host Host.
             * @param user User.
             * @param password Password.
             * @param database Database.
             * @param port Port.
             */
            void open(const std::string& host, const std::string& user, const std::string& password, const std::string& database, int port=5432);
            /**
             * @brief Close the open database.
             */
            void close();
            /**
             * @brief Check if the database is empty.
             * @return bool
             */
            [[nodiscard]] bool empty() const;
            /**
             * @brief Validate an SQL statement.
             * @param query Query to validate.
             * @return bool True if valid.
             */
            [[nodiscard]] bool validate(const std::string& query) const;
            /**
             * @brief Get the last insertion.
             * @return std::int64_t Last insertion.
             */
            [[nodiscard]] std::int64_t get_last_insertion() const;
            /**
             * @brief Constructor.
             */
            postgresql_database() = default;
            /**
             * @brief Constructor.
             * @param host Host.
             * @param user User.
             * @param password Password.
             * @param database Database.
             * @param port Port.
             */
            postgresql_database(const std::string& host, const std::string& user, const std::string& password, const std::string& database, int port=5432);
            /**
             * @brief Destructor.
             */
            ~postgresql_database();
    };
#endif
}

#ifdef LIMHAMN_DATABASE_IMPL
inline std::string limhamn::database::remove_non_utf8(const std::string& input) {
#ifdef LIMHAMN_DATABASE_ICONV
    iconv_t cd = iconv_open("UTF-8//IGNORE", "UTF-8");
    if (cd == reinterpret_cast<iconv_t>(-1)) {
        throw std::runtime_error("iconv_open failed");
    }

    std::vector<char> output(input.size() * 2);
    char* inbuf = const_cast<char*>(input.data());
    size_t inbytesleft = input.size();
    char* outbuf = output.data();
    size_t outbytesleft = output.size();

    while (inbytesleft > 0) {
        size_t result = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
        if (result == static_cast<size_t>(-1)) {
            if (errno == EILSEQ || errno == EINVAL) {
                ++inbuf;
                --inbytesleft;
            } else if (errno == E2BIG) {
                size_t used = output.size() - outbytesleft;
                output.resize(output.size() * 2);
                outbuf = output.data() + used;
                outbytesleft = output.size() - used;
            } else {
                iconv_close(cd);
                return "";
            }
        }
    }

    iconv_close(cd);
    return {output.data(), output.size() - outbytesleft};
#else
    return input;
#endif
}

#ifdef LIMHAMN_DATABASE_SQLITE3
/**
 * @brief Bind parameters to a prepared statement.
 *
 * @param stmt Statement to bind to.
 * @param index Index of the parameter.
 * @param value Value to bind.
 * @param args Remaining parameters.
 */
template<typename T, typename... Args>
inline void limhamn::database::sqlite3_database::bind_parameters(sqlite3_stmt* stmt, int index, T value, Args... args) {
    if constexpr (std::is_same_v<T, int> ||
                  std::is_same_v<T, int64_t> ||
                  std::is_same_v<T, double> ||
                  std::is_same_v<T, std::string> ||
                  std::is_same_v<T, const char*>) {
        bind_parameter(stmt, index, value);
    }

    bind_parameters(stmt, index + 1, args...);
}


inline void limhamn::database::sqlite3_database::bind_parameters(sqlite3_stmt* stmt, int index) {
    static_cast<void>(stmt);
    static_cast<void>(index);
}
inline void limhamn::database::sqlite3_database::bind_parameter(sqlite3_stmt* stmt, int index, int value) {
#ifdef SDB_ENABLE_PRINTDEBUG
    std::cerr << "Binding int: " << value << " to index: " << index << "\n";
#endif
    sqlite3_bind_int(stmt, index, value);
}

inline void limhamn::database::sqlite3_database::bind_parameter(sqlite3_stmt* stmt, int index, int64_t value) {
#ifdef SDB_ENABLE_PRINTDEBUG
    std::cerr << "Binding int: " << value << " to index: " << index << "\n";
#endif
    sqlite3_bind_int64(stmt, index, value);
}

inline void limhamn::database::sqlite3_database::bind_parameter(sqlite3_stmt* stmt, int index, double value) {
#ifdef SDB_ENABLE_PRINTDEBUG
    std::cerr << "Binding double: " << value << " to index: " << index << "\n";
#endif
    sqlite3_bind_double(stmt, index, value);
}

inline void limhamn::database::sqlite3_database::bind_parameter(sqlite3_stmt* stmt, int index, const std::string& value) {
#ifdef SDB_ENABLE_PRINTDEBUG
    std::cerr << "Binding string: " << value << " to index: " << index << "\n";
#endif
    sqlite3_bind_text(stmt, index, remove_non_utf8(value).c_str(), -1, SQLITE_TRANSIENT);
}

inline void limhamn::database::sqlite3_database::bind_parameter(sqlite3_stmt* stmt, int index, const char* value) {
#ifdef SDB_ENABLE_PRINTDEBUG
    std::cerr << "Binding string: " << value << " to index: " << index << "\n";
#endif
    sqlite3_bind_text(stmt, index, remove_non_utf8(value).c_str(), -1, SQLITE_TRANSIENT);
}

inline int limhamn::database::sqlite3_database::callback(void* data, int argc, char** argv, char** name) {
    static_cast<void>(static_cast<void*>(data));

    std::unordered_map<std::string, std::string> map{};
    for (int i{0}; i < argc; i++) {
        map[name[i]] = argv[i] ? argv[i] : "";
    }

    tmp.push_back(std::move(map));

    return 0;
}

inline limhamn::database::sqlite3_database::sqlite3_database(const std::string& database) {
    if (sqlite3_open(database.c_str(), &this->sqlite3_db)) {
        return;
    }

    this->database = database;
    this->is_good = true;
}

inline limhamn::database::sqlite3_database::sqlite3_database() {
    this->is_good = false;
}

inline void limhamn::database::sqlite3_database::open(const std::string& database) {
    if (this->is_good) {
        return;
    }

    if (sqlite3_open(database.c_str(), &this->sqlite3_db)) {
        return;
    }

    this->is_good = true;
}

inline bool limhamn::database::sqlite3_database::exec(const std::string& query) const {
    if (!this->is_good) {
        return false;
    }

    if (!this->validate(query)) {
        throw std::runtime_error{"Invalid SQL statement in database file '" + this->database + "': " + query + "\n"};
    }

    char* err{};

    int ret = sqlite3_exec(sqlite3_db, query.c_str(), nullptr, nullptr, &err);

    if (ret != SQLITE_OK) {
        sqlite3_free(err);
        return false;
    }

    return true;
}

inline bool limhamn::database::sqlite3_database::validate(const std::string& query) const {
    if (!this->is_good) {
        return false;
    }

    sqlite3_stmt* stmt;

    int ret = sqlite3_prepare_v2(sqlite3_db, query.c_str(), -1, &stmt, nullptr);

    if (ret != SQLITE_OK) {
        return false;
    }

    sqlite3_finalize(stmt);

    return true;
}

inline std::vector<std::unordered_map<std::string, std::string>> limhamn::database::sqlite3_database::query(const std::string& query) const {
    if (!this->is_good) {
        return {};
    }

    if (!this->validate(query)) {
        throw std::runtime_error{"Invalid SQL statement: " + query + "\n"};
    }

    char* err{};

    int status = sqlite3_exec(sqlite3_db, query.c_str(), limhamn::database::sqlite3_database::callback, nullptr, &err);

    if (status != SQLITE_OK) {
        sqlite3_free(err);
        return {};
    }

    return std::move(tmp);
}

inline bool limhamn::database::sqlite3_database::good() const {
    return this->is_good;
}

inline bool limhamn::database::sqlite3_database::is_open() const {
    return this->good();
}

inline void limhamn::database::sqlite3_database::close() {
    if (this->is_good) {
        sqlite3_close(this->sqlite3_db);
        this->is_good = false;
    }
}

inline bool limhamn::database::sqlite3_database::empty() const {
    return std::ifstream(this->database).peek() == std::ifstream::traits_type::eof();
}

inline limhamn::database::sqlite3_database::~sqlite3_database() {
    if (this->is_good) {
        this->close();
    }
}

inline std::int64_t limhamn::database::sqlite3_database::get_last_insertion() const {
    if (!this->is_good) {
        return -1;
    }

    return sqlite3_last_insert_rowid(this->sqlite3_db);
}

template <typename... Args>
inline bool limhamn::database::sqlite3_database::exec(const std::string& query, Args... args) {
    sqlite3_stmt* stmt;

    std::string nq{};
    for (size_t i = 0; i < query.size(); ++i) {
        if (query[i] == '$' && i + 1 < query.size() && isdigit(query[i + 1])) {
            nq += '?';
            while (i + 1 < query.size() && isdigit(query[i + 1])) {
                ++i;
            }
        } else {
            nq += query[i];
        }
    }

    if (sqlite3_prepare_v2(sqlite3_db, nq.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement\n";
        return false;
    }

    bind_parameters(stmt, 1, args...);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to step statement" << sqlite3_errmsg(sqlite3_db) << "\n";
        return false;
    }

    sqlite3_finalize(stmt);

    return true;
}
template <typename... Args>
std::vector<std::unordered_map<std::string, std::string>> limhamn::database::sqlite3_database::query(const std::string& query, Args... args) {
    if (!this->is_good) {
        return {};
    }

    std::string nq{};
    for (size_t i = 0; i < query.size(); ++i) {
        if (query[i] == '$' && i + 1 < query.size() && isdigit(query[i + 1])) {
            nq += '?';
            while (i + 1 < query.size() && isdigit(query[i + 1])) {
                ++i;
            }
        } else {
            nq += query[i];
        }
    }

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(sqlite3_db, nq.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return {};
    }

    bind_parameters(stmt, 1, args...);

    std::vector<std::unordered_map<std::string, std::string>> result;
    while ((sqlite3_step(stmt)) == SQLITE_ROW) {
        std::unordered_map<std::string, std::string> row;
        for (int i = 0; i < sqlite3_column_count(stmt); ++i) {
            row[sqlite3_column_name(stmt, i)] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
        }
        result.push_back(std::move(row));
    }

    sqlite3_finalize(stmt);
    return result;
}
#endif
#ifdef LIMHAMN_DATABASE_POSTGRESQL
inline limhamn::database::postgresql_database::postgresql_database(const std::string& host,
    const std::string& user, const std::string& password, const std::string& database, int port) {

    this->open(host, user, password, database, port);
}

inline limhamn::database::postgresql_database::~postgresql_database() {
    if (this->is_good) {
        this->close();
    }
}

inline void limhamn::database::postgresql_database::open(const std::string& host,
        const std::string& user, const std::string& password, const std::string& database, int port) {
    if (this->is_good) {
        return;
    }

    this->host = host;
    this->user = user;
    this->password = password;
    this->database = database;
    this->port = port;

    const std::string conninfo = ("host=" + host + " user=" + user + " password=" + password + " dbname=" + database + " port=" + std::to_string(port));
    this->pg_conn = PQconnectdb(conninfo.c_str());

    if (PQstatus(pg_conn) != CONNECTION_OK || !pg_conn) {
        PQfinish(pg_conn);
        return;
    }

    PQsetNoticeProcessor(pg_conn, [](void*, const char* message) {
        static_cast<void>(message);
    }, nullptr);

    this->is_good = true;
}

inline bool limhamn::database::postgresql_database::exec(const std::string& query) const {
    if (!this->is_good) {
        return false;
    }

    if (PQstatus(pg_conn) != CONNECTION_OK) {
        throw std::runtime_error{"Connection to database failed: " + std::string(PQerrorMessage(pg_conn))};
    }

    if (!this->validate(query)) {
        throw std::runtime_error{"Invalid SQL statement in database '" + this->database + "': " + query + "\n"};
    }

    PGresult* res = PQexec(pg_conn, query.c_str());

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        PQclear(res);
        return false;
    }

    PQclear(res);
    return true;
}

inline bool limhamn::database::postgresql_database::validate(const std::string& query) const {
    if (!this->is_good) {
        return false;
    }

    PGresult* res = PQprepare(pg_conn, "", query.c_str(), 0, nullptr);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        PQclear(res);
        return false;
    }

    PQclear(res);
    return true;
}

inline std::vector<std::unordered_map<std::string, std::string>> limhamn::database::postgresql_database::query(const std::string& query) const {
    if (!this->is_good) {
        return {};
    }

    if (!this->validate(query)) {
        throw std::runtime_error{"Invalid SQL statement: " + query + "\n"};
    }

    PGresult* res = PQexec(pg_conn, query.c_str());

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return {};
    }

    std::vector<std::unordered_map<std::string, std::string>> result;
    int nrows = PQntuples(res);
    int nfields = PQnfields(res);

    for (int i = 0; i < nrows; ++i) {
        std::unordered_map<std::string, std::string> row;
        for (int j = 0; j < nfields; ++j) {
            row[PQfname(res, j)] = PQgetvalue(res, i, j);
        }
        result.push_back(std::move(row));
    }

    PQclear(res);
    return result;
}

inline bool limhamn::database::postgresql_database::good() const {
    return this->is_good;
}

inline bool limhamn::database::postgresql_database::is_open() const {
    return this->good();
}

inline void limhamn::database::postgresql_database::close() {
    if (this->is_good) {
        PQfinish(this->pg_conn);
        this->is_good = false;
    }
}

inline bool limhamn::database::postgresql_database::empty() const {
    if (!this->is_good) {
        return true;
    }

    const char* query = "SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = 'public';";
    PGresult* res = PQexec(pg_conn, query);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return true;
    }

    bool is_empty = std::stoi(PQgetvalue(res, 0, 0)) == 0;
    PQclear(res);
    return is_empty;
}

inline std::int64_t limhamn::database::postgresql_database::get_last_insertion() const {
    if (!this->is_good) {
        return -1;
    }

    const char* query = "SELECT LASTVAL();";
    PGresult* res = PQexec(pg_conn, query);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return -1;
    }

    std::int64_t last_insertion = std::stoll(PQgetvalue(res, 0, 0));
    PQclear(res);
    return last_insertion;
}

template <typename T>
inline std::string limhamn::database::postgresql_database::to_string(const T& value) {
    if constexpr (std::is_same_v<T, std::string>) {
        return value;
    } else if constexpr (std::is_same_v<T, const char*>) {
        return std::string(value);
    } else {
        return std::to_string(value);
    }
}

template <typename... Args>
inline bool limhamn::database::postgresql_database::exec(const std::string& query, Args... args) {
    if (!this->is_good) {
        return false;
    }

    int i{1};
    std::string nq{};
    for (char ch : query) {
        if (ch == '?') {
            nq += "$" + std::to_string(i++);
        } else {
            nq += ch;
        }
    }

    std::vector<std::string> str{remove_non_utf8(to_string(args))...};
    std::vector<const char*> param_v{};
    param_v.reserve(str.size());
    for (const std::string& s : str) {
#ifdef SDB_ENABLE_PRINTDEBUG
        std::cerr << "Binding string: " << s << "\n";
#endif
        param_v.push_back(s.c_str());
    }

    PGresult* res = PQexecParams(pg_conn, nq.c_str(), static_cast<int>(param_v.size()), nullptr, param_v.data(), nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        PQclear(res);
        return false;
    }

    PQclear(res);
    return true;
}

template <typename... Args>
inline std::vector<std::unordered_map<std::string, std::string>> limhamn::database::postgresql_database::query(const std::string& query, Args... args) {
    if (!this->is_good) {
        return {};
    }

    int n{1};
    std::string nq{};
    for (char ch : query) {
        if (ch == '?') {
            nq += "$" + std::to_string(n++);
        } else {
            nq += ch;
        }
    }

    std::vector<std::string> str{to_string(args)...};
    std::vector<const char*> param_v{};
    param_v.reserve(str.size());
    for (const std::string& s : str) {
        param_v.push_back(s.c_str());
    }

    PGresult* res = PQexecParams(pg_conn, nq.c_str(), static_cast<int>(param_v.size()), nullptr, param_v.data(), nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return {};
    }

    std::vector<std::unordered_map<std::string, std::string>> result;
    int nrows = PQntuples(res);
    int nfields = PQnfields(res);

    for (int i = 0; i < nrows; ++i) {
        std::unordered_map<std::string, std::string> row;
        for (int j = 0; j < nfields; ++j) {
            row[PQfname(res, j)] = PQgetvalue(res, i, j);
        }
        result.push_back(std::move(row));
    }

    PQclear(res);
    return result;
}
#endif // LIMHAMN_DATABASE_POSTGRESQL
#endif // LIMHAMN_DATABASE_IMPL
