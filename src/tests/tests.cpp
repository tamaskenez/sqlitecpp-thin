#include SQLITECPPTHIN_HEADER

#include "gtest/gtest.h"

// Macro/template magic to convert function results to uniform std::expected<T, sqlite::error>, no matter whether the
// test is compiled with the expected or exception style lib.
//
// Use `AS_BOOL_LIKE(X)` on non-void functions that are expected to pass.
//
//     auto result = AS_BOOL_LIKE(f());
//     ASSERT_TRUE(result);
//     ... *result ... // Use the result.
//
// Use `AS_BOOL_LIKE_FROM_VOID(X)` on void functions that are expected to pass:
//
//     auto result = AS_BOOL_LIKE_VOID(f());
//     ASSERT_TRUE(result);
//
// Use `AS_EXPECTED(T, X)` on non-void functions that are expected to fail.
//
//     auto result = AS_EXPECTED(result_type_of_f, f());
//     ASSERT_FALSE(result);
//     ASSERT_EQ(result.error().errcode, ...); // result.error() will be of type std::expected<T, sqlite::error>
//

#if SQLITECPPTHIN_EXPECTED

template<class T>
std::expected<T, sqlite::error> as_expected(std::expected<T, sqlite::error>&& x)
{
    return std::move(x);
}

template<class T>
std::expected<T, sqlite::error> as_expected(std::expected<T, sqlite::current_error>&& x)
{
    return x.error();
}

  #define AS_BOOL_LIKE(X) X
  #define AS_BOOL_LIKE_FROM_VOID(X) X
  #define AS_EXPECTED(T, X) as_expected<T>(X)

#elif SQLITECPPTHIN_EXCEPTION
  #define AS_BOOL_LIKE(X) std::optional(X)
  #define AS_BOOL_LIKE_FROM_VOID(X) (X, true)
  #define AS_EXPECTED(T, X)                      \
      [&]() -> std::expected<T, sqlite::error> { \
        try {                                    \
            return X;                            \
        } catch (sqlite::exception & e) {        \
            return std::unexpected(e.error());   \
        }                                        \
      }()
#else
  #error
#endif

TEST(open, memory_const_char)
{
    auto db = AS_BOOL_LIKE(sqlite::open(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));
    ASSERT_TRUE(db);
    auto r2 = AS_BOOL_LIKE_FROM_VOID(db->close());
    ASSERT_TRUE(r2);
}

TEST(open, memory_string)
{
    auto db = AS_BOOL_LIKE(sqlite::open(std::string(":memory:"), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));
    ASSERT_TRUE(db);
    auto r2 = AS_BOOL_LIKE_FROM_VOID(db->close());
    ASSERT_TRUE(r2);
}

TEST(open, memory_path)
{
    auto db = AS_BOOL_LIKE(sqlite::open(std::filesystem::path(":memory:"), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));
    ASSERT_TRUE(db);
    auto r2 = AS_BOOL_LIKE_FROM_VOID(db->close());
    ASSERT_TRUE(r2);
}

TEST(open, fail_on_invalid_file)
{
    auto db = AS_EXPECTED(
      sqlite::database, sqlite::open("/nosuch/file/or/directory", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)
    );
    ASSERT_FALSE(db);
}

TEST(open, moves)
{
    // Open 2 databases and remember raw handles.
    auto db1 =
      AS_BOOL_LIKE(sqlite::open(std::filesystem::path(":memory:"), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));
    ASSERT_TRUE(db1);
    auto* db1_handle = db1->handle();
    auto db2 =
      AS_BOOL_LIKE(sqlite::open(std::filesystem::path(":memory:"), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));
    ASSERT_TRUE(db2);
    auto* db2_handle = db2->handle();

    // Move constructor and assignment.
    sqlite::database db3(std::move(*db1));
    db2 = std::move(db3);

    // db1 moved out
    ASSERT_FALSE(db1->handle());
    // db2 contains db1_handle
    ASSERT_EQ(db2->handle(), db1_handle);
    // db3 moved out
    ASSERT_FALSE(db3.handle());

    auto rdb2 = AS_BOOL_LIKE_FROM_VOID(db2->close());
    ASSERT_TRUE(rdb2);
    ASSERT_FALSE(db2->handle());

    // db2 was destructed here: `db2 = std::move(db3);`, close must fail.
    ASSERT_EQ(sqlite3_close(db2_handle), SQLITE_MISUSE);
}

TEST(open, database_destructor)
{
    sqlite3* db_handle{};
    {
        auto db = AS_BOOL_LIKE(sqlite::open(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));
        ASSERT_TRUE(db);
        db_handle = db->handle();
        // db destructed.
    }
    ASSERT_TRUE(db_handle);
    // close must fail.
    ASSERT_EQ(sqlite3_close(db_handle), SQLITE_MISUSE);
}
