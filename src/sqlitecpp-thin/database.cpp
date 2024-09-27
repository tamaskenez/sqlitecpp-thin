#include "sqlite3.hpp"

#include "common.hpp"

#define RETURN_UNEXPECTED_ON_ERROR(X)              \
    if (int rc = (X)) {                            \
        RETURN_UNEXPECTED(current_error(rc, _db)); \
    }

namespace sqlite
{

database::database(sqlite3* db)
    : _db(db)
{
}

database::database(database&& y)
    : _db(y._db)
{
    y._db = nullptr;
}

database& database::operator=(database&& y)
{
    auto was_this = MOVE(*this);
    std::swap(_db, y._db);
    return *this;
}

database::~database()
{
    if (_db) {
        sqlite3_close_v2(_db);
    }
}

expected<void, current_error> database::close()
{
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_close(_db))
    _db = nullptr;
    RETURN_VOID;
}

namespace
{
int row_callback_caller(void* void_row_callback_ptr, int num_rows, char** column_texts, char** column_names)
{
    auto* row_callback_ptr = reinterpret_cast<const database::row_callback_t*>(void_row_callback_ptr);
    return (*row_callback_ptr)(
      span<const char* const>(column_texts, size_t(num_rows)), span<const char* const>(column_names, size_t(num_rows))
    );
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
    RETURN_VOID;
}

expected<int, current_error> database::exec_changes(string_like_zt sql)
{
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, nullptr))
    return sqlite3_changes(_db);
}

string database::errmsg() const
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

expected<statement, current_error> database::prepare(string_like sql)
{
    sqlite3_stmt* stmt{};
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_prepare_v2(
      _db,
      sql.c_str(),
      sql.size() ? int(*sql.size()) : -1, // Note: zero terminator doesn't need to be included in the size.
      &stmt,
      nullptr
    ))
    return statement(stmt);
}

} // namespace sqlite
