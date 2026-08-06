#pragma once

#include <sqlite3.h>

#include <concepts>
#include <expected>
#include <filesystem>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace sqlite
{

// Reduce syntactic noise.
using std::byte;
using std::expected;
using std::function;
using std::optional;
using std::span;
using std::string;
using std::string_view;
namespace fs = std::filesystem;

// String-like, zero terminated function parameter.
struct string_like_zt {
    string_like_zt(const string& s)
        : _c_str(s.c_str())
    {
    }
    string_like_zt(const char* cc)
        : _c_str(cc)
    {
    }

    [[nodiscard]] const char* c_str() const
    {
        return _c_str;
    }

private:
    const char* _c_str;
};

// String-like function parameter when zero-termination is not needed.
struct string_like {
    string_like(std::nullptr_t)
        : _c_str(nullptr)
    {
    }

    // This one accepts `const char*`. We need to constrain it with an inline concept otherwise it gets selected for
    // string literals, too.
    string_like(std::same_as<const char*> auto cc)
        : _c_str(cc)
    {
    }

    template<size_t N>
    string_like(const char (&string_literal)[N])
        : _c_str(string_literal)
        , _size(N)
    {
    }

    string_like(const string& s)
        : _c_str(s.c_str())
        , _size(s.size())
    {
    }

    string_like(string_view sv)
        : _c_str(sv.data())
        , _size(sv.size())
    {
    }

    [[nodiscard]] const char* c_str() const
    {
        return _c_str;
    }

    [[nodiscard]] const optional<size_t>& size() const
    {
        return _size;
    }

private:
    const char* _c_str;
    optional<size_t> _size;
};

// `error` stores all error fields from the database in the struct, as opposed to the other similar struct
// `current_error` below.
struct error {
    int errcode = SQLITE_OK;
    int extended_errcode = SQLITE_OK;
    string errmsg;
    int error_offset = -1;

    [[nodiscard]] string format() const;
};

// `current_error` stores only the primary error code and the database handle.
// All other fields are retrieved from the database object only when requested. That's why `current_error` must be
// processed immediately, it should not be stored, since subsequent operations could change the error fields on the
// database.
class current_error
{
public:
    current_error(int errcode, sqlite3* db);
    explicit current_error(sqlite3* db);

    [[nodiscard]] int errcode() const;
    [[nodiscard]] int extended_errcode() const;
    [[nodiscard]] const char* errmsg() const;
    [[nodiscard]] int error_offset() const;

    [[nodiscard]] error get_error() const;

    [[nodiscard]] string format() const;

private:
    int _errcode = SQLITE_OK;
    sqlite3* _db = nullptr;
};

enum class step_result {
    row = SQLITE_ROW,
    done = SQLITE_DONE
};

enum class datatype {
    integer = SQLITE_INTEGER,
    float_ = SQLITE_FLOAT,
    blob = SQLITE_BLOB,
    null = SQLITE_NULL,
    text = SQLITE_TEXT
};

// Return the name of the C macro (e.g. "SQLITE_BUSY") for the error code. `nullptr` if the error code is not valid.
const char* errcode_macro_name(int rc);

class statement
{
public:
    explicit statement(sqlite3_stmt* stmt);

    // `statement` is move-only
    statement(const statement&) = delete;
    statement(statement&& y) noexcept;
    statement& operator=(const statement&) = delete;
    statement& operator=(statement&& y) noexcept;

    // sqlite3_finalize().
    ~statement();

    [[nodiscard]] sqlite3_stmt* handle() const
    {
        return _stmt;
    }

    // Note about the memory management of `bind_blob` and `bind_text` functions:
    //
    // The `bind_blob` and `bind_text` functions assume the passed memory block will be kept alive until the next rebind
    // or reset. This corresponds to the `SQLITE_STATIC` flag on the C interface. In addition to that, the functions
    // taking `const void*` and `const char*` might be provided with a C-style deleter function.
    //
    // The `bind_blob_copy` and `bind_text_copy` functions make a copy of the memory block before
    // returning, which corresponds to the `SQLITE_TRANSIENT` flag on the C interface.
    //
    // Note that while `bind_blob(int, string&&)` and `bind_text(int, string&&)` are deleted, in C++ it's still easier
    // to accidentally provide a temporary object to these functions then it is in C. For example:
    //
    //     stmt.bind_text(0, std::string_view(produce_some_string(...))); # Compiles OK but use-after-free!!!
    //

    // sqlite3_bind_blob().
    [[nodiscard]] expected<void, current_error>
    bind_blob(int index, const void* ptr, int size, void (*deleter)(void*) = SQLITE_STATIC);

    // sqlite3_bind_blob64().
    [[nodiscard]] expected<void, current_error>
    bind_blob(int index, const void* ptr, size_t size, void (*deleter)(void*) = SQLITE_STATIC);

    // sqlite3_bind_blob(..., SQLITE3_TRANSIENT).
    [[nodiscard]] expected<void, current_error> bind_blob_copy(int index, const void* ptr, int size);

    // sqlite3_bind_blob64(..., SQLITE3_TRANSIENT).
    [[nodiscard]] expected<void, current_error> bind_blob_copy(int index, const void* ptr, size_t size);

    // C++ - style bind_blob functions
    // WARNING: unlike `sqlite3_bind_blob()`: empty `span` or `string_view` bind an empty blob and never a NULL value,
    // even if `data() == nullptr`!

    // sqlite3_bind_blob64(..., SQLITE3_STATIC).
    [[nodiscard]] expected<void, current_error> bind_blob(int index, span<const std::byte> bytes);
    [[nodiscard]] expected<void, current_error> bind_blob(int index, string_view sv);

    // sqlite3_bind_blob64(..., SQLITE3_TRANSIENT).
    [[nodiscard]] expected<void, current_error> bind_blob_copy(int index, span<const std::byte> bytes);
    [[nodiscard]] expected<void, current_error> bind_blob_copy(int index, string_view sv);
    [[nodiscard]] expected<void, current_error> bind_blob_copy(int index, string&& s);

    // Remove the `string&&` overload: temporary strings must always be copied.
    [[nodiscard]] expected<void, current_error> bind_blob(int index, string&& s) = delete;

    // sqlite3_bind_double().
    [[nodiscard]] expected<void, current_error> bind_double(int index, double d);

    // sqlite3_bind_int().
    [[nodiscard]] expected<void, current_error> bind_int(int index, int i);

    // sqlite3_bind_int64().
    [[nodiscard]] expected<void, current_error> bind_int(int index, int64_t i);

    // Accept all integers which can safely be converted to `int` (and exclude `int` itself)
    template<class T>
        requires(
          !std::same_as<T, int>
          && ((std::signed_integral<T> && sizeof(T) <= sizeof(int)) || (std::unsigned_integral<T> && sizeof(T) + 1 <= sizeof(int)))
        )
    [[nodiscard]] expected<void, current_error> bind_int(int index, T i)
    {
        return bind_int(index, static_cast<int>(i));
    }

    // Accept all integers which can safely be converted to `int64_t` (and exclude `int64_t` itself)
    template<class T>
        requires(
          !std::same_as<T, int64_t>
          && ((std::signed_integral<T> && sizeof(T) <= sizeof(int64_t)) || (std::unsigned_integral<T> && sizeof(T) + 1 <= sizeof(int64_t)))
        )
    [[nodiscard]] expected<void, current_error> bind_int(int index, T i)
    {
        return bind_int(index, static_cast<int64_t>(i));
    }

    // sqlite3_bind_null().
    [[nodiscard]] expected<void, current_error> bind_null(int index);

    // bind_text functions: please note that "The result of expressions involving strings with embedded NULs is
    // undefined."

    // sqlite3_bind_text().
    [[nodiscard]] expected<void, current_error>
    bind_text(int index, const char* ptr, int size, void (*deleter)(void*) = SQLITE_STATIC);

    // sqlite3_bind_text64().
    [[nodiscard]] expected<void, current_error>
    bind_text(int index, const char* ptr, size_t size, void (*deleter)(void*) = SQLITE_STATIC);

    // sqlite3_bind_text(..., SQLITE_TRANSIENT).
    [[nodiscard]] expected<void, current_error> bind_text_copy(int index, const char* ptr, int size);

    // sqlite3_bind_text64(..., SQLITE_TRANSIENT).
    [[nodiscard]] expected<void, current_error> bind_text_copy(int index, const char* ptr, size_t size);

    // C-style bind_text functions for zero-terminated c-strings.
    [[nodiscard]] expected<void, current_error>
    bind_text(int index, const char* ptr, void (*deleter)(void*) = SQLITE_STATIC);
    [[nodiscard]] expected<void, current_error> bind_text_copy(int index, const char* ptr);

    // C++ - style bind_text functions.
    // WARNING: unlike `sqlite3_bind_text()`: empty `string` or `string_view` bind an empty text and never a NULL value,
    // even if `data() == nullptr`!

    // sqlite3_bind_text64(..., SQL_STATIC).
    [[nodiscard]] expected<void, current_error> bind_text(int index, string_view sv);
    // sqlite3_bind_text64(..., SQL_TRANSIENT).
    [[nodiscard]] expected<void, current_error> bind_text_copy(int index, string_view sv);
    [[nodiscard]] expected<void, current_error> bind_text_copy(int index, string&& s);

    // Remove the `string&&` overload: temporary strings must always be copied.
    [[nodiscard]] expected<void, current_error> bind_text(int index, string&& s) = delete;

    // sqlite3_bind_zeroblob().
    [[nodiscard]] expected<void, current_error> bind_zeroblob(int index, int size);

    // sqlite3_bind_zeroblob64().
    [[nodiscard]] expected<void, current_error> bind_zeroblob(int index, size_t size);

    expected<step_result, current_error> step();

    // Normal column getters:
    // Return null-like values (empty blob, 0.0, 0, "") for NULL values.

    // sqlite3_column_blob() and sqlite3_column_bytes().
    expected<span<const byte>, current_error> column_blob(int col);

    // sqlite3_column_double().
    double column_double(int col);

    // sqlite3_column_int().
    int column_int(int col);

    // sqlite3_column_int64().
    int64_t column_int64(int col);

    // sqlite3_column_text() and sqlite3_column_bytes()
    // The `data()` of the returned value can be used as `const char*` since it's zero-terminated.
    expected<string_view, current_error> column_text(int col);

    // sqlite3_column_type().
    expected<datatype, current_error> column_type(int col);

    // Optional column getters: actual NULL is returned as std::nullopt, error as std::unexpected.
    expected<optional<span<const byte>>, current_error> column_blob_opt(int col);
    expected<optional<string_view>, current_error> column_text_opt(int col);

    // sqlite3_reset().
    expected<void, current_error> reset();

    // sqlite3_db_handle().
    [[nodiscard]] sqlite3* db_handle() const;

    // Convenience compound functions:

    // sqlite3_step(), verify it's SQLITE_DONE, then return sqlite3_changes().
    expected<int, current_error> step_done_changes();

private:
    sqlite3_stmt* _stmt;
};

class database
{
public:
    explicit database(sqlite3* db);

    // `database` is move-only.
    database(const database&) = delete;
    database(database&& y) noexcept;
    database& operator=(const database&) = delete;
    database& operator=(database&& y) noexcept;

    // Force-close the underlying database using sqlite3_close_v2().
    ~database();

    [[nodiscard]] sqlite3* handle() const
    {
        return _db;
    }

    // Call sqlite_close() which returns a result code. On error, the database is not closed.
    [[nodiscard]] expected<void, current_error> close();

    // sqlite3_exec().
    using row_callback_t = function<int(span<const char* const> column_texts, span<const char* const> column_names)>;
    [[nodiscard]] expected<void, current_error> exec(string_like_zt sql, const row_callback_t& row_callback = {});

    // sqlite3_exec(), then return sqlite3_changes().
    [[nodiscard]] expected<int, current_error> exec_changes(string_like_zt sql);

    // sqlite3_errmsg().
    [[nodiscard]] const char* errmsg() const;

    // sqlite3_errcode().
    [[nodiscard]] int errcode() const;

    // sqlite3_extended_errcode().
    [[nodiscard]] int extended_errcode() const;

    // sqlite3_offset().
    [[nodiscard]] int error_offset() const;

    // sqlite3_prepare().
    // Note: if `sql` contains no SQL (if the input is an empty string or a comment), unexpected will be returned
    // with SQLITE_OK as the error. This is similar to sqlite3_prepare_v2() which returns SQLITE_OK but the statement
    // pointer is set to NULL.
    expected<statement, current_error> prepare(string_like sql);

private:
    sqlite3* _db;
};

expected<database, error> open(const string& filename, int flags);
expected<database, error> open(const char* filename, int flags);
expected<database, error> open(const fs::path& filename, int flags);

} // namespace sqlite
