#pragma once

#include "sqlite3.hpp"

#include <utility>

#define MOVE(X) std::move(X)

namespace sqlite
{

using std::nullopt;

#if SQLITECPPTHIN_EXPECTED
  #define RETURN_UNEXPECTED(X) return std::unexpected(X)
  #define RETURN_VOID \
      return {}
#elif SQLITECPPTHIN_EXCEPTION
  #define RETURN_UNEXPECTED(X) throw(sqlite::exception(X))
  #define RETURN_VOID return
#else
  #error Either SQLITECPPTHIN_EXCEPTION or SQLITECPPTHIN_EXPECTED must be defined to 1.
#endif
} // namespace sqlite
