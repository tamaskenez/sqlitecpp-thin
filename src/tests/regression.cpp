#include "test_util.hpp"

namespace
{

// A row of "null-like but not NULL" values (the ones the sentinel-based column getters cannot tell
// apart from a failure), plus a row of genuine NULLs and a row of ordinary non-zero values.
sqlite::database make_db()
{
    auto db = sqlite::open_v2(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE).value();
    EXPECT_TRUE(db.exec("CREATE TABLE t (id INTEGER, i INTEGER, i64 INTEGER, d REAL, s TEXT, b BLOB)"));
    EXPECT_TRUE(db.exec("INSERT INTO t VALUES (0, 0, 0, 0.0, '', x'')"));
    EXPECT_TRUE(db.exec("INSERT INTO t VALUES (1, NULL, NULL, NULL, NULL, NULL)"));
    EXPECT_TRUE(db.exec("INSERT INTO t VALUES (2, 42, 42, 42.5, 'text', x'0102')"));
    return db;
}

sqlite::statement select_row(sqlite::database& db, int id)
{
    auto stmt = db.prepare_v2("SELECT i, i64, d, s, b FROM t WHERE id = " + std::to_string(id)).value();
    EXPECT_EQ(stmt.step(), sqlite::step_result::row);
    return stmt;
}

// Make an unrelated operation fail so that sqlite3_errcode() on the connection is left non-OK. Any
// failing call on the same connection between step() and the column reads will do.
void poison_connection_errcode(sqlite::database& db)
{
    EXPECT_FALSE(db.prepare_v2("SELECT * FROM no_such_table"));
    EXPECT_EQ(db.errcode(), SQLITE_ERROR);
}

} // namespace

// --- Column getters -------------------------------------------------------------------------

// The column getters return the value when it is non-sentinel and otherwise consult
// sqlite3_errcode(). That reports the last *failed* call on the connection, not on the statement,
// and column access neither sets nor clears it. So a stored 0 read after an unrelated failure is
// reported as that unrelated failure.
TEST(regression, column_getter_false_error_after_unrelated_failure)
{
    auto db = make_db();
    auto stmt = select_row(db, 0);

    poison_connection_errcode(db);

    auto i = stmt.column_int(0);
    EXPECT_EQ(i, 0);
}

// The same thing, self-inflicted and without any unrelated statement: an out-of-range column index
// leaves SQLITE_RANGE on the connection, which then poisons every later read.
TEST(regression, column_getter_false_error_after_out_of_range_index)
{
    auto db = make_db();
    auto stmt = select_row(db, 0);

    EXPECT_FALSE(stmt.column_int(99)) << "an out-of-range column index should be an error";

    auto i = stmt.column_int(0);
    EXPECT_EQ(i, 0);
}

// Nearly every getter is affected, because each has a zero-like sentinel: 0 for the integer
// getters, 0.0 for column_double(), and a null blob pointer for x''. column_text() escapes on this
// row only because SQLite hands back a non-null pointer for an empty string - it falls into the
// same trap when the value is a real NULL.
TEST(regression, column_getters_survive_poisoned_errcode)
{
    auto db = make_db();
    auto stmt = select_row(db, 0);

    poison_connection_errcode(db);

    auto i = stmt.column_int(0);
    auto i64 = stmt.column_int64(1);
    auto d = stmt.column_double(2);
    auto s = stmt.column_text(3);
    auto b = stmt.column_blob(4);

    EXPECT_EQ(i, 0);
    EXPECT_EQ(i64, 0);
    EXPECT_EQ(d, 0.0);
    EXPECT_TRUE(s.has_value() && s->empty());
    EXPECT_TRUE(b.has_value() && b->empty());
}

TEST(regression, optional_column_getters_survive_poisoned_errcode)
{
    auto db = make_db();
    auto stmt = select_row(db, 0);

    poison_connection_errcode(db);

    auto i = stmt.column_int_opt(0);
    auto i64 = stmt.column_int64_opt(1);
    auto d = stmt.column_double_opt(2);
    auto s = stmt.column_text_opt(3);
    auto b = stmt.column_blob_opt(4);

    EXPECT_TRUE(i.has_value() && *i == 0);
    EXPECT_TRUE(i64.has_value() && *i64 == 0);
    EXPECT_TRUE(d.has_value() && *d == 0.0);
    EXPECT_TRUE(s.has_value() && s->has_value() && (*s)->empty());
    EXPECT_TRUE(b.has_value() && b->has_value() && (*b)->empty());
}

// Coverage gap that let the defects above through: no test ever read back a stored zero or empty
// value. This one passes today, on an unpoisoned connection.
TEST(regression, column_getters_read_back_zero_and_empty_values)
{
    auto db = make_db();
    auto stmt = select_row(db, 0);

    auto i = stmt.column_int(0);
    auto i64 = stmt.column_int64(1);
    auto d = stmt.column_double(2);
    auto s = stmt.column_text(3);
    auto b = stmt.column_blob(4);

    ASSERT_TRUE(s.has_value());
    ASSERT_TRUE(b.has_value());

    EXPECT_EQ(i, 0);
    EXPECT_EQ(i64, 0);
    EXPECT_EQ(d, 0.0);
    EXPECT_TRUE(s->empty());
    EXPECT_TRUE(b->empty());

    EXPECT_EQ(stmt.column_type(0), sqlite::datatype::integer);
    EXPECT_EQ(stmt.column_type(3), sqlite::datatype::text);
    EXPECT_EQ(stmt.column_type(4), sqlite::datatype::blob);
}

// The `_opt` getters had no coverage at all. For a stored zero/empty value they must return an
// engaged optional. This half works today.
TEST(regression, optional_column_getters_read_back_zero_and_empty_values)
{
    auto db = make_db();
    auto stmt = select_row(db, 0); // zero-like but not NULL

    auto i = stmt.column_int_opt(0);
    auto i64 = stmt.column_int64_opt(1);
    auto d = stmt.column_double_opt(2);
    auto s = stmt.column_text_opt(3);
    auto b = stmt.column_blob_opt(4);

    ASSERT_TRUE(i.has_value());
    ASSERT_TRUE(i64.has_value());
    ASSERT_TRUE(d.has_value());
    ASSERT_TRUE(s.has_value());
    ASSERT_TRUE(b.has_value());

    ASSERT_EQ(*i, 0);
    ASSERT_EQ(*i64, 0);
    ASSERT_EQ(*d, 0.0);
    ASSERT_TRUE(s->has_value());
    ASSERT_TRUE(b->has_value());

    EXPECT_TRUE((*s)->empty());
    EXPECT_TRUE((*b)->empty());
}

// column_type() checks `if (int rc = sqlite3_errcode(db))` without exempting SQLITE_ROW, unlike the
// two macros at the top of statement.cpp which check `rc != SQLITE_OK && rc != SQLITE_ROW`. Right
// after a step() that returned a row, sqlite3_errcode() *is* SQLITE_ROW (100), which is truthy. So
// column_type() reports a bogus SQLITE_ROW error every time the value is NULL - the one case the
// function has to get right, since NULL is the only type that reaches that branch.
TEST(regression, column_type_of_a_null_value)
{
    auto db = make_db();
    auto stmt = select_row(db, 1); // all NULL

    auto t = stmt.column_type(0);
    ASSERT_TRUE(t.has_value()) << "column_type() failed on a NULL value: " << t.error().format();
    EXPECT_EQ(*t, sqlite::datatype::null);
}

// Fallout from the defect above, and the more damaging half of it. The `_opt` getters decide
// between nullopt and a value with `if (column_type(col) == datatype::null)`. When column_type()
// returns an error that comparison is simply false, so the NULL branch never runs (and the error is
// swallowed). Every `_opt` getter therefore hands back an engaged optional holding 0 / "" / an
// empty span for a genuine NULL - defeating the entire point of the overload set.
TEST(regression, optional_column_getters_return_nullopt_for_null)
{
    auto db = make_db();
    auto stmt = select_row(db, 1); // all NULL

    auto i = stmt.column_int_opt(0);
    auto i64 = stmt.column_int64_opt(1);
    auto d = stmt.column_double_opt(2);
    auto s = stmt.column_text_opt(3);
    auto b = stmt.column_blob_opt(4);

    ASSERT_TRUE(i.has_value());
    ASSERT_TRUE(i64.has_value());
    ASSERT_TRUE(d.has_value());
    ASSERT_TRUE(s.has_value());
    ASSERT_TRUE(b.has_value());

    EXPECT_FALSE(i->has_value());
    EXPECT_FALSE(i64->has_value());
    EXPECT_FALSE(d->has_value());
    EXPECT_FALSE(s->has_value()) << "SQL NULL was reported as the value \"" << **s << "\"";
    EXPECT_FALSE(b->has_value()) << "SQL NULL was reported as a blob of " << (*b)->size() << " bytes";
}

// column_text() promises more than the other getters: "the `data()` of the returned value can be
// used as `const char*` since it's zero-terminated", and NULL is documented to read back as "". Both
// hold for a stored empty string, because SQLite hands back a pointer to a zero-terminated "" for
// it. This one passes today.
TEST(regression, column_text_of_an_empty_string_is_zero_terminated)
{
    auto db = make_db();
    auto stmt = select_row(db, 0); // s = ''

    auto s = stmt.column_text(3);
    ASSERT_TRUE(s.has_value());
    EXPECT_TRUE(s->empty());
    ASSERT_NE(s->data(), nullptr);
    EXPECT_STREQ(s->data(), "");
}

// For a real NULL, sqlite3_column_text() answers nullptr, and column_text() falls through to
// `return {}`. That value-initializes the string_view, so data() is null - not the promised "".
// A caller taking the documented route, printf("%s", sv.data()) or strlen(sv.data()), reads through
// a null pointer on the first NULL column it meets.
//
// `return string_view("")` is enough. column_blob()'s `return {}` needs no change: an empty span is
// exactly what that one advertises.
TEST(regression, column_text_of_null_is_zero_terminated)
{
    auto db = make_db();
    auto stmt = select_row(db, 1); // s IS NULL

    auto s = stmt.column_text(3);
    ASSERT_TRUE(s.has_value());
    EXPECT_TRUE(s->empty());
    ASSERT_NE(s->data(), nullptr) << "column_text() of a NULL returned a string_view with data() == nullptr";
    EXPECT_STREQ(s->data(), "");
}

// --- prepare() ------------------------------------------------------------------------------

// sqlite3_prepare_v2() returns SQLITE_OK with *ppStmt == nullptr for SQL that contains no
// statement. prepare() currently reports that as success and hands out a `statement` wrapping a
// null handle. Using it is then fatal: step() returns SQLITE_MISUSE, db_handle() returns nullptr,
// and the resulting current_error trips the assert in error.cpp - or, with NDEBUG, formats the
// misleading message "out of memory".
//
// Either prepare() should fail, or it should hand back something usable. It must not do this.
TEST(regression, prepare_of_statementless_sql)
{
    auto db = make_db();
    for (const char* sql : {"", "   ", "\n\t ", ";", ";;", "-- just a comment"}) {
        auto stmt = db.prepare_v2(sql);
        if (stmt) {
            EXPECT_NE(stmt->handle(), nullptr)
              << "prepare(\"" << sql << "\") reported success but the statement handle is null";
        }
    }
}

// A default-constructed string_view has data() == nullptr and size() == 0, so prepare() passed a
// null pointer to sqlite3_prepare_v2(), which answered SQLITE_MISUSE *without* setting the
// connection error code.
//
// This was fixed by correctly handling the empty string_view case (data = nullptr but size = 0) and translating
// it into an empty string for sqlite_prepare_v2.
TEST(regression, prepare_of_empty_string_view)
{
    auto db = make_db();
    auto stmt = db.prepare_v2(std::string_view{});
    ASSERT_FALSE(stmt);
    ASSERT_EQ(stmt.error().errcode(), SQLITE_OK);
}

TEST(regression, prepare_of_nullptr_string)
{
    auto db = make_db();
    {
        auto stmt = db.prepare_v2(nullptr);
        ASSERT_FALSE(stmt);
        ASSERT_EQ(stmt.error().errcode(), SQLITE_MISUSE);
    }
    {
        const char* nullptr_str = nullptr;
        auto stmt = db.prepare_v2(nullptr_str);
        ASSERT_FALSE(stmt);
        ASSERT_EQ(stmt.error().errcode(), SQLITE_MISUSE);
    }
}

// --- exec() ---------------------------------------------------------------------------------

// row_callback_caller() is a C callback: sqlite3_exec() calls it from inside its own C frames, and
// it forwards to a std::function supplied by the caller. Nothing stops that from throwing, and an
// exception then unwinds through SQLite. That is undefined behaviour on its own, but it also does
// visible damage: SQLite's cleanup is skipped, so the statement sqlite3_exec() prepared internally
// is never finalized. The connection is left holding it, close() fails with SQLITE_BUSY ("unable to
// close due to unfinalized statements or unfinished backups"), and ~database() then trips its
// assert.
//
// Not hypothetical: CHECK() in test_util.hpp throws, and database_test.cpp's exec_with_row_callback
// uses CHECK() inside an exec() row callback. A failing CHECK there would wreck the connection
// instead of reporting a test failure.
//
// The fix is to stop the exception at the C boundary. This test assumes it gets caught and turned
// into a non-zero return, so exec() reports SQLITE_ABORT like any other callback that asks to stop.
// If row_callback_caller() is made `noexcept` instead - std::terminate() rather than UB - then this
// test should become an EXPECT_DEATH.
//
// WARNING: today this test does not fail, it aborts the process in the destructor of `db`, after
// printing its failed expectations. Run it on its own.
TEST(regression, exec_row_callback_that_throws)
{
    auto db = make_db();

    bool escaped = false;
    try {
        auto exec_result =
          db.exec("SELECT i FROM t", [](std::span<const char* const>, std::span<const char* const>) -> int {
              throw std::runtime_error("from the row callback");
          });
        EXPECT_FALSE(exec_result) << "a row callback that could not run should be reported as an error";
    } catch (const std::runtime_error&) {
        escaped = true;
    }
    EXPECT_FALSE(escaped) << "the exception unwound through sqlite3_exec()'s C frames";

    // The real damage, and the part that outlives the exception.
    EXPECT_TRUE(db.close()) << "the connection was left holding the statement sqlite3_exec() failed to finalize";
}

// --- error formatting -----------------------------------------------------------------------

// current_error keeps the connection handle only when the error was really recorded on it,
// otherwise it stores nullptr. In that state errmsg() answers nullptr - which is not documented in
// the header, and which get_error() feeds straight into `error::errmsg`, a std::string.
//
// The states with no connection are not exotic, they are documented outcomes: prepare_v2() of SQL
// that contains no statement, prepare_v2(nullptr), and step_expect_done_then_changes() when the
// statement produced a row. This test pins the precondition, without touching get_error().
TEST(regression, current_error_without_a_connection_has_no_errmsg)
{
    auto db = make_db();
    {
        auto stmt = db.prepare_v2("-- just a comment");
        ASSERT_FALSE(stmt);
        EXPECT_EQ(stmt.error().errcode(), SQLITE_OK);
        EXPECT_EQ(stmt.error().errmsg(), nullptr);
    }
    {
        auto stmt = db.prepare_v2(nullptr);
        ASSERT_FALSE(stmt);
        EXPECT_EQ(stmt.error().errcode(), SQLITE_MISUSE);
        EXPECT_EQ(stmt.error().errmsg(), nullptr);
    }
    {
        auto stmt = db.prepare_v2("SELECT i FROM t").value();
        auto changes = stmt.step_expect_done_then_changes();
        ASSERT_FALSE(changes);
        EXPECT_EQ(changes.error().errcode(), SQLITE_ROW);
        EXPECT_EQ(changes.error().errmsg(), nullptr);
    }
}

// Constructing a std::string from nullptr is undefined behaviour, and in practice it segfaults in
// strlen. So every one of those states is a crash waiting for the caller to ask what went wrong -
// through get_error(), or through format(), which is get_error() plus a bit of text.
//
// Three tests above produce exactly this error object and get away with it only because they never
// format it. column_type_of_a_null_value has it worse: its ASSERT_TRUE message calls format(), so if
// that assertion ever fired it would crash instead of reporting the failure.
//
// `errmsg() ? errmsg() : ""` in get_error() fixes it. errmsg() returning nullptr is worth a line in
// the header either way.
//
// WARNING: today this test does not fail, it segfaults. Run it on its own.
TEST(regression, current_error_without_a_connection_can_be_formatted)
{
    auto db = make_db();
    auto stmt = db.prepare_v2("-- just a comment");
    ASSERT_FALSE(stmt);

    const auto e = stmt.error().get_error();
    EXPECT_EQ(e.errcode, SQLITE_OK);
    EXPECT_TRUE(e.errmsg.empty());

    EXPECT_FALSE(stmt.error().format().empty());
}

// sqlite3_errstr() is documented to return NULL for a result code that has no text message
// available, and its message table really does hold NULL entries: SQLITE_INTERNAL, SQLITE_EMPTY,
// SQLITE_FORMAT and - unless SQLITE_DISABLE_LFS - SQLITE_NOLFS. The amalgamation currently
// substitutes a fallback "unknown error" for those, so in practice no NULL escapes, but that is an
// implementation detail rather than the contract and the library links against whatever SQLite the
// consumer has. error::format() is right to handle NULL, and this test deliberately asserts nothing
// about which of its branches runs.
//
// What it does pin down is error::format()'s own contract: a non-empty string carrying the message,
// for any result code - including the ones with no message available, and values that are not
// result codes at all.
TEST(regression, error_format_handles_codes_without_a_message)
{
    for (int rc :
         {SQLITE_OK,
          SQLITE_ERROR,
          SQLITE_INTERNAL,
          SQLITE_EMPTY,
          SQLITE_FORMAT,
          SQLITE_NOLFS,
          SQLITE_CONSTRAINT_NOTNULL,
          -12345,
          999999}) {
        const sqlite::error e{.errcode = rc, .extended_errcode = rc, .errmsg = "the message", .error_offset = -1};
        const auto s = e.format();
        EXPECT_FALSE(s.empty()) << "errcode " << rc;
        EXPECT_NE(s.find("the message"), std::string::npos) << "errcode " << rc << ": " << s;
    }
}

TEST(regression, error_format_renders_the_offset)
{
    const sqlite::error with_offset{
      .errcode = SQLITE_ERROR, .extended_errcode = SQLITE_ERROR, .errmsg = "the message", .error_offset = 7
    };
    EXPECT_NE(with_offset.format().find("at char 7"), std::string::npos) << with_offset.format();

    const sqlite::error without_offset{
      .errcode = SQLITE_ERROR, .extended_errcode = SQLITE_ERROR, .errmsg = "the message", .error_offset = -1
    };
    EXPECT_EQ(without_offset.format().find("at char"), std::string::npos) << without_offset.format();
}

// errcode_macro_name() is the lookup that really is documented to answer nullptr, and format() has
// to cope with that too.
TEST(regression, errcode_macro_name_returns_null_for_unknown_codes)
{
    EXPECT_EQ(sqlite::errcode_macro_name(-12345), nullptr);
    EXPECT_EQ(sqlite::errcode_macro_name(999999), nullptr);
    EXPECT_STREQ(sqlite::errcode_macro_name(SQLITE_ERROR), "SQLITE_ERROR");
}

TEST(regression, string_like_size_with_literal)
{
    const auto sl = sqlite::string_like("abc");
    EXPECT_EQ(sl.size(), 3);
    EXPECT_EQ(strncmp(sl.data(), "abc", 3), 0);
}

TEST(regression, string_like_size_with_array)
{
    constexpr char arr[3] = {'a', 'b', 'c'};
    const auto sl = sqlite::string_like(arr);
    EXPECT_EQ(sl.size(), 3);
    EXPECT_EQ(strncmp(sl.data(), "abc", 3), 0);
}
