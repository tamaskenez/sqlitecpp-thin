#include "include/sqlitecpp-thin/sqlite3.hpp"

#include "common.hpp"

#define RETURN_UNEXPECTED_ON_ERROR(X)                      \
    if (int rc = (X)) {                                    \
        return unexpected(current_error(rc, db_handle())); \
    }

#define RETURN_UNEXPECTED_ON_SQLITE3_ERRCODE                                 \
    auto* db = db_handle();                                                  \
    if (int rc = sqlite3_errcode(db); rc != SQLITE_OK && rc != SQLITE_ROW) { \
        return unexpected(current_error(rc, db));                            \
    }

#define RETURN_UNEXPECTED_ON_ERROR_OR_NULLOPT_ON_NULL \
    auto* db = db_handle();                           \
    int rc = sqlite3_errcode(db);                     \
    if (rc != SQLITE_OK && rc != SQLITE_ROW) {        \
        return unexpected(current_error(rc, db));     \
    }                                                 \
    if (column_type(col) == datatype::null) {         \
        return nullopt;                               \
    }

namespace sqlite
{

statement::statement(sqlite3_stmt* stmt)
    : _stmt(stmt)
{
}

statement::statement(statement&& y) noexcept
    : _stmt(y._stmt)
{
    y._stmt = nullptr;
}

statement::~statement()
{
    sqlite3_finalize(_stmt);
}

statement& statement::operator=(statement&& y) noexcept
{
    auto was_this = MOVE(*this);
    std::swap(_stmt, y._stmt);
    return *this;
}

sqlite3* statement::db_handle() const
{
    return sqlite3_db_handle(_stmt);
}

expected<step_result, current_error> statement::step()
{
    const int rc = sqlite3_step(_stmt);
    switch (rc) {
    case SQLITE_ROW:
        return step_result::row;
    case SQLITE_DONE:
        return step_result::done;
    default:
        return unexpected(current_error(rc, db_handle()));
    }
}

expected<int, current_error> statement::step_done_changes()
{
    const auto sr = step();
    if (!sr) {
        return unexpected(sr.error());
    }
    if (*sr != step_result::done) {
        // Report SQLITE_ROW as error.
        return unexpected(current_error(db_handle()));
    }
    return sqlite3_changes(db_handle());
}

expected<span<const byte>, current_error> statement::column_blob(int col)
{
    if (auto* p = sqlite3_column_blob(_stmt, col)) {
        return span(static_cast<const byte*>(p), iicast<size_t>(sqlite3_column_bytes(_stmt, col)));
    }
    RETURN_UNEXPECTED_ON_SQLITE3_ERRCODE
    return {};
}

expected<double, current_error> statement::column_double(int col)
{
    if (auto d = sqlite3_column_double(_stmt, col); d != 0.0) {
        return d;
    }
    RETURN_UNEXPECTED_ON_SQLITE3_ERRCODE
    return 0.0;
}

expected<int, current_error> statement::column_int(int col)
{
    if (auto i = sqlite3_column_int(_stmt, col); i != 0) {
        return i;
    }
    RETURN_UNEXPECTED_ON_SQLITE3_ERRCODE
    return 0;
}

expected<int64_t, current_error> statement::column_int64(int col)
{
    if (auto i = sqlite3_column_int64(_stmt, col); i != 0) {
        return i;
    }
    RETURN_UNEXPECTED_ON_SQLITE3_ERRCODE
    return 0;
}

expected<string_view, current_error> statement::column_text(int col)
{
    if (auto* p = sqlite3_column_text(_stmt, col)) {
        return string_view(reinterpret_cast<const char*>(p), iicast<size_t>(sqlite3_column_bytes(_stmt, col)));
    }
    RETURN_UNEXPECTED_ON_SQLITE3_ERRCODE
    return {};
}

expected<optional<span<const byte>>, current_error> statement::column_blob_opt(int col)
{
    if (auto* p = sqlite3_column_blob(_stmt, col)) {
        return span(static_cast<const byte*>(p), iicast<size_t>(sqlite3_column_bytes(_stmt, col)));
    }
    RETURN_UNEXPECTED_ON_ERROR_OR_NULLOPT_ON_NULL
    return optional(span<const byte>());
}

expected<optional<double>, current_error> statement::column_double_opt(int col)
{
    if (auto d = sqlite3_column_double(_stmt, col); d != 0.0) {
        return d;
    }
    RETURN_UNEXPECTED_ON_ERROR_OR_NULLOPT_ON_NULL
    return 0.0;
}

expected<optional<int>, current_error> statement::column_int_opt(int col)
{
    if (auto i = sqlite3_column_int(_stmt, col); i != 0) {
        return i;
    }
    RETURN_UNEXPECTED_ON_ERROR_OR_NULLOPT_ON_NULL
    return 0;
}

expected<optional<int64_t>, current_error> statement::column_int64_opt(int col)
{
    if (auto i = sqlite3_column_int64(_stmt, col); i != 0) {
        return i;
    }
    RETURN_UNEXPECTED_ON_ERROR_OR_NULLOPT_ON_NULL
    return 0;
}

expected<optional<string_view>, current_error> statement::column_text_opt(int col)
{
    if (auto* p = sqlite3_column_text(_stmt, col)) {
        return string_view(reinterpret_cast<const char*>(p), iicast<size_t>(sqlite3_column_bytes(_stmt, col)));
    }
    RETURN_UNEXPECTED_ON_ERROR_OR_NULLOPT_ON_NULL
    return optional(string_view());
}

expected<datatype, current_error> statement::column_type(int col)
{
    auto r = sqlite3_column_type(_stmt, col);
    switch (r) {
    case SQLITE_INTEGER:
    case SQLITE_FLOAT:
    case SQLITE_TEXT:
    case SQLITE_BLOB:
        return static_cast<datatype>(r);
    default:
        assert(r == SQLITE_NULL); // sqlite3_column_type is not expected to return anything else.
        {
            auto* db = db_handle();
            if (int rc = sqlite3_errcode(db)) {
                return unexpected(current_error(rc, db));
            }
        }
        return datatype::null;
    }
}

expected<void, current_error> statement::reset()
{
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_reset(_stmt))
    return {};
}

expected<void, current_error> statement::bind_blob(int index, const void* ptr, int size, void (*deleter)(void*))
{
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_bind_blob(_stmt, index, ptr, size, deleter))
    return {};
}

expected<void, current_error> statement::bind_blob(int index, const void* ptr, size_t size, void (*deleter)(void*))
{
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_bind_blob64(_stmt, index, ptr, size, deleter))
    return {};
}

expected<void, current_error> statement::bind_blob_copy(int index, const void* ptr, int size)
{
    return bind_blob(index, ptr, size, SQLITE_TRANSIENT);
}

expected<void, current_error> statement::bind_blob_copy(int index, const void* ptr, size_t size)
{
    return bind_blob(index, ptr, size, SQLITE_TRANSIENT);
}

expected<void, current_error> statement::bind_blob(int index, span<const std::byte> bytes)
{
    static constexpr byte k_byte{};
    return bind_blob(index, bytes.empty() ? &k_byte : bytes.data(), bytes.size());
}

expected<void, current_error> statement::bind_blob(int index, string_view sv)
{
    static constexpr char k_char{};
    return bind_blob(index, sv.empty() ? &k_char : sv.data(), sv.size());
}

expected<void, current_error> statement::bind_blob_copy(int index, span<const std::byte> bytes)
{
    static constexpr byte k_byte{};
    return bind_blob(index, bytes.empty() ? &k_byte : bytes.data(), bytes.size(), SQLITE_TRANSIENT);
}

expected<void, current_error> statement::bind_blob_copy(int index, string_view sv)
{
    static constexpr char k_char{};
    return bind_blob(index, sv.empty() ? &k_char : sv.data(), sv.size(), SQLITE_TRANSIENT);
}

expected<void, current_error> statement::bind_blob_copy(int index, string&& s)
{
    static constexpr char k_char{};
    return bind_blob(index, s.empty() ? &k_char : s.data(), s.size(), SQLITE_TRANSIENT);
}

expected<void, current_error> statement::bind_double(int index, double d)
{
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_bind_double(_stmt, index, d))
    return {};
}

expected<void, current_error> statement::bind_int(int index, int i)
{
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_bind_int(_stmt, index, i))
    return {};
}

expected<void, current_error> statement::bind_int(int index, int64_t i)
{
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_bind_int64(_stmt, index, i))
    return {};
}

expected<void, current_error> statement::bind_null(int index)
{
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_bind_null(_stmt, index))
    return {};
}

expected<void, current_error> statement::bind_text(int index, const char* ptr, int size, void (*deleter)(void*))
{
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_bind_text(_stmt, index, ptr, size, deleter))
    return {};
}

expected<void, current_error> statement::bind_text(int index, const char* ptr, size_t size, void (*deleter)(void*))
{
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_bind_text64(_stmt, index, ptr, size, deleter, SQLITE_UTF8))
    return {};
}

expected<void, current_error> statement::bind_text_copy(int index, const char* ptr, int size)
{
    return bind_text(index, ptr, size, SQLITE_TRANSIENT);
}

expected<void, current_error> statement::bind_text_copy(int index, const char* ptr, size_t size)
{
    return bind_text(index, ptr, size, SQLITE_TRANSIENT);
}

expected<void, current_error> statement::bind_text(int index, const char* ptr, void (*deleter)(void*))
{
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_bind_text(_stmt, index, ptr, -1, deleter))
    return {};
}

expected<void, current_error> statement::bind_text_copy(int index, const char* ptr)
{
    return bind_text(index, ptr, SQLITE_TRANSIENT);
}

expected<void, current_error> statement::bind_text(int index, string_view sv)
{
    static constexpr char k_char{};
    return bind_text(index, sv.empty() ? &k_char : sv.data(), sv.size());
}

expected<void, current_error> statement::bind_text_copy(int index, string_view sv)
{
    static constexpr char k_char{};
    return bind_text(index, sv.empty() ? &k_char : sv.data(), sv.size(), SQLITE_TRANSIENT);
}

expected<void, current_error> statement::bind_text_copy(int index, string&& s)
{
    static constexpr char k_char{};
    return bind_text(index, s.empty() ? &k_char : s.data(), s.size(), SQLITE_TRANSIENT);
}

expected<void, current_error> statement::bind_zeroblob(int index, int size)
{
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_bind_zeroblob(_stmt, index, size))
    return {};
}

expected<void, current_error> statement::bind_zeroblob(int index, size_t size)
{
    RETURN_UNEXPECTED_ON_ERROR(sqlite3_bind_zeroblob64(_stmt, index, size))
    return {};
}

} // namespace sqlite
