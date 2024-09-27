#include "sqlite3.hpp"

#include <cassert>

#ifdef HAS_FORMAT
  #include <format>
#else
  #include "fmt/core.h"
#endif

namespace format
{
#ifdef HAS_FORMAT
using std::format;
#else
using fmt::format;
#endif
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
    assert(_errcode == sqlite3_errcode(_db));
    return _errcode;
}

int current_error::extended_errcode() const
{
    assert(_errcode == sqlite3_errcode(_db));
    return sqlite3_extended_errcode(_db);
}

const char* current_error::errmsg() const
{
    assert(_errcode == sqlite3_errcode(_db));
    return sqlite3_errmsg(_db);
}

int current_error::error_offset() const
{
    assert(_errcode == sqlite3_errcode(_db));
    return sqlite3_error_offset(_db);
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

    auto maybe_offset = [this]() {
        return error_offset >= 0 ? format::format(" at char {}", error_offset) : string();
    };

    const char* errstr = sqlite3_errstr(extended_errcode);
    if (errstr) {
        return format::format(
          "{} ({}){}: \"{}\"", errstr, macro_name_or_number(extended_errcode), maybe_offset(), errmsg
        );
    }

    if (errcode != extended_errcode) {
        if (errstr = sqlite3_errstr(errcode); errstr) {
            return format::format(
              "{} ({}){}: \"{}\", extended_errcode ({}) is invalid",
              errstr,
              macro_name_or_number(errcode),
              maybe_offset(),
              errmsg,
              macro_name_or_number(extended_errcode)
            );
        }
    }

    // Both errcode and extended_errcode are invalid, which means `sqlite3_errstr()` returns nullptr
    return format::format(
      "Unknown SQL error{}: \"{}\", both errcode ({}) and extended_errcode ({}) are invalid",
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
