#pragma once

#include "include/sqlitecpp-thin/sqlite3.hpp"

#include <cassert>
#include <concepts>
#include <expected>
#include <optional>
#include <utility>

#define MOVE(X) std::move(X)

namespace sqlite
{

using std::nullopt;
using std::unexpected;

template<class To, class From>
    requires std::integral<To> && std::integral<From>
constexpr To iicast(From f)
{
    assert(std::in_range<To>(f));
    return static_cast<To>(f);
}

} // namespace sqlite
