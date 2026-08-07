#include "include/sqlitecpp-thin/sqlite3.hpp"

#include <cassert>
#include <format>

namespace format
{
using std::format;
} // namespace format

namespace sqlite
{
current_error::current_error(int errcode, sqlite3* db)
    : _errcode(errcode)
    // Not all errors are set on the database, for example SQLITE_MISUSE sometimes isn't.
    // Indicate it by not storing db.
    // Note: rare corner case not handled: for example, an SQLITE_MISUSE is already set on the database while
    // encountering a second SQLITE_MISUSE which is not set on the database. We will mistakenly store the db for the
    // second SQLITE_MISUSE, too.
    , _db(db && sqlite3_errcode(db) == errcode ? db : nullptr)
{
}

int current_error::errcode() const
{
    assert(
      !_db || _errcode == sqlite3_errcode(_db)
    ); // Probable reason: another operation was performed on the database after this current_error was obtained.
    return _errcode;
}

int current_error::extended_errcode() const
{
    if (_db) {
        assert(
          _errcode == sqlite3_errcode(_db)
        ); // Probable reason: another operation was performed on the database after this current_error was obtained.
        return sqlite3_extended_errcode(_db);
    }
    return _errcode;
}

const char* current_error::errmsg() const
{
    if (_db) {
        assert(
          _errcode == sqlite3_errcode(_db)
        ); // Probable reason: another operation was performed on the database after this current_error was obtained.
        return sqlite3_errmsg(_db);
    }
    return nullptr;
}

int current_error::error_offset() const
{
    if (_db) {
        assert(
          _errcode == sqlite3_errcode(_db)
        ); // Probable reason: another operation was performed on the database after this current_error was obtained.
        return sqlite3_error_offset(_db);
    }
    return -1;
}

error current_error::get_error() const
{
    return sqlite::error{
      .errcode = _errcode, .extended_errcode = extended_errcode(), .errmsg = errmsg(), .error_offset = error_offset()
    };
}

string error::format() const
{
    // Sample:
    //
    //     SQL logic error (SQLITE_ERROR) at char 16: "no such column: foo"
    //

    auto macro_name_or_number = [](int x) {
        auto s = sqlite::errcode_macro_name(x);
        if (s) {
            return string(s);
        }
        return std::to_string(x);
    };

    auto maybe_offset = [this] {
        return error_offset >= 0 ? format::format(" at char {}", error_offset) : string();
    };

    const char* errstr = sqlite3_errstr(errcode);
    if (errstr) {
        return format::format(
          "{} ({}){}: \"{}\"", errstr, macro_name_or_number(extended_errcode), maybe_offset(), errmsg
        );
    }

    // Both errcode is invalid, which means `sqlite3_errstr()` returns nullptr
    return format::format(
      "Unknown SQL error{}: \"{}\", no errstr for errcode {}, extended_errcode: {}",
      maybe_offset(),
      errmsg,
      macro_name_or_number(errcode),
      macro_name_or_number(extended_errcode)
    );
}

string current_error::format() const
{
    return get_error().format();
}

} // namespace sqlite
