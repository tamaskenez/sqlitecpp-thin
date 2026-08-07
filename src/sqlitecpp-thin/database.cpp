#include "include/sqlitecpp-thin/sqlite3.hpp"

#include "common.hpp"

#define RETURN_UNEXPECTED_ON_ERROR(X)              \
    if (int rc = (X)) {                            \
        return unexpected(current_error(rc, _db)); \
    }

namespace sqlite
{

database::database(sqlite3* db)
    : _db(db)
{
}

database::database(database&& y) noexcept
    : _db(y._db)
{
    y._db = nullptr;
}

database& database::operator=(database&& y) noexcept
{
    auto was_this = MOVE(*this);
    std::swap(_db, y._db);
    return *this;
}

database::~database()
{
    if (_db) {
        if (const auto rc = sqlite3_close(_db); rc != SQLITE_OK) {
            assert(false); // Probable reason: "If the database connection is associated with unfinalized prepared
                           // statements, BLOB handlers, and/or unfinished sqlite3_backup objects then sqlite3_close()
                           // will leave the database connection open and return SQLITE_BUSY."

            // Fallback:
            //
            // If sqlite3_close_v2() is called with unfinalized prepared statements, unclosed BLOB handlers, and/or
            // unfinished sqlite3_backups, it returns SQLITE_OK regardless, but instead of deallocating the database
            // connection immediately, it marks the database connection as an unusable "zombie" and makes arrangements
            // to automatically deallocate the database connection after all prepared statements are finalized, all BLOB
            // handles are closed, and all backups have finished.
            sqlite3_close_v2(_db);
        }
    }
}

expected<void, current_error> database::close()
{
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_close(_db))
    _db = nullptr;
    return {};
}

namespace
{
int row_callback_caller(void* void_row_callback_ptr, int num_columns, char** column_texts, char** column_names)
{
    auto* row_callback_ptr = static_cast<const database::row_callback_t*>(void_row_callback_ptr);
    const auto nc = iicast<size_t>(num_columns);
    try {
        return (*row_callback_ptr)(
          // The (deprecated) `PRAGMA empty_result_callbacks` allows `column_texts` to be nullptr.
          column_texts ? span<const char* const>(column_texts, nc) : span<const char* const>(),
          span<const char* const>(column_names, nc)
        );
    } catch (...) {
        return SQLITE_ABORT;
    }
}
} // namespace

expected<void, current_error> database::exec(string_like_zt sql, const row_callback_t& row_callback)
{
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_exec(
      _db,
      sql.c_str(),
      row_callback ? &row_callback_caller : nullptr,
      const_cast<row_callback_t*>(&row_callback),
      nullptr
    ))
    return {};
}

expected<int, current_error> database::exec_then_changes(string_like_zt sql)
{
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, nullptr))
    return sqlite3_changes(_db);
}

const char* database::errmsg() const
{
    return sqlite3_errmsg(_db);
}

int database::errcode() const
{
    return sqlite3_errcode(_db);
}

int database::extended_errcode() const
{
    return sqlite3_extended_errcode(_db);
}

int database::error_offset() const
{
    return sqlite3_error_offset(_db);
}

expected<statement, current_error> database::prepare_v2(string_like sql, const char** pzTail)
{
    sqlite3_stmt* stmt{};
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_prepare_v2(
      _db,
      sql.size() == 0 ? "" : sql.data(),
      sql.size() && std::cmp_less_equal(*sql.size(), INT_MAX)
        ? iicast<int>(*sql.size())
        : -1, // Note: zero terminator doesn't need to be included in the size.
      &stmt,
      pzTail
    ))
    if (stmt) {
        return statement(stmt);
    }
    // We get here when result of `sqlite3_prepare_v2` was SQLITE_OK but `stmt == nullptr`. It happens when:
    // "If the input text contains no SQL (if the input is an empty string or a comment) then *ppStmt is set to NULL."
    return unexpected(current_error(SQLITE_OK, nullptr));
}

} // namespace sqlite
