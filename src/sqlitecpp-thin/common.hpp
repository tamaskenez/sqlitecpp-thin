#pragma once

#include "sqlite3.hpp"

#include <expected>
#include <utility>

#define MOVE(X) std::move(X)

namespace sqlite
{

using std::nullopt;

#if defined SQLITECPPTHIN_EXPECTED && SQLITECPPTHIN_EXPECTED
  #define RETURN_UNEXPECTED(X) return std::unexpected(X)
  #define RETURN_VOID \
      return {}
#elif defined SQLITECPPTHIN_EXCEPTION && SQLITECPPTHIN_EXCEPTION
  #define RETURN_UNEXPECTED(X) throw(X)
  #define RETURN_VOID return
#else
  #error Either SQLITECPPTHIN_EXCEPTION or SQLITECPPTHIN_EXPECTED must be defined to 1.
#endif

error create_error_for_db(sqlite3* db);

} // namespace sqlite
