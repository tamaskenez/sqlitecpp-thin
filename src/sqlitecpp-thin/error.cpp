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
    , _db(db)
{
}

current_error::current_error(sqlite3* db)
    : _errcode(sqlite3_errcode(db))
    , _db(db)
{
}

int current_error::errcode() const
{
    assert(_errcode == SQLITE_MISUSE || _errcode == sqlite3_errcode(_db));
    return _errcode;
}

int current_error::extended_errcode() const
{
    if (_errcode == sqlite3_errcode(_db)) {
        return sqlite3_extended_errcode(_db);
    }
    assert(_errcode == SQLITE_MISUSE);
    return _errcode;
}

const char* current_error::errmsg() const
{
    if (_errcode == sqlite3_errcode(_db)) {
        return sqlite3_errmsg(_db);
    }
    assert(_errcode == SQLITE_MISUSE);
    return nullptr;
}

int current_error::error_offset() const
{
    if (_errcode == sqlite3_errcode(_db)) {
        return sqlite3_error_offset(_db);
    }
    assert(_errcode == SQLITE_MISUSE);
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
