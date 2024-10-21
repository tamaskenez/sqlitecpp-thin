#include "test_util.hpp"

TEST(database, prepare_query_text_modes)
{
    for (bool with_size : {false, true}) {
        auto db = sqlite::open(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE).value();
        ASSERT_TRUE(db.exec("CREATE TABLE foo (a INTEGER)"));

        ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(2)"));
        ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(2)"));
        ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(22)"));
        ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(22)"));
        ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(22)"));
        ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(222)"));
        ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(222)"));
        ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(222)"));
        ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(222)"));

        const std::string query = "SELECT count(1) FROM foo WHERE a = 22";
        auto stmt = (with_size ? db.prepare(query) : db.prepare(query.c_str())).value();

        ASSERT_EQ(stmt.step(), sqlite::step_result::row);
        ASSERT_EQ(stmt.column_int(0), 3);
    }
}

TEST(database, exec_with_row_callback)
{
    const std::vector<std::pair<int, std::string>> data = {
      {2, "two"  },
      {3, "three"},
      {4, "four" }
    };
    for (bool first_two_rows_only : {false, true}) {
        auto db = sqlite::open(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE).value();
        ASSERT_TRUE(db.exec("CREATE TABLE foo (a INTEGER, s TEXT)"));
        for (auto& d : data) {
            ASSERT_TRUE(db.exec("INSERT INTO foo(a, s) VALUES(" + std::to_string(d.first) + ", '" + d.second + "')"));
        }
        size_t counter = 0;
        auto exec_result = db.exec(
          "SELECT * from foo",
          [&counter, &data, first_two_rows_only](
            std::span<const char* const> column_texts, std::span<const char* const> column_names
          ) -> int {
              CHECK(counter < data.size());
              auto& d = data[counter++];
              CHECK(column_texts.size() == 2);
              CHECK(column_texts[0] == std::to_string(d.first));
              CHECK(column_texts[1] == d.second);
              CHECK(column_names.size() == 2);
              CHECK(column_names[0] == std::string("a"));
              CHECK(column_names[1] == std::string("s"));
              return first_two_rows_only && counter == 2 ? 1 : 0;
          }
        );
        if (first_two_rows_only) {
            ASSERT_EQ(counter, 2);
            ASSERT_FALSE(exec_result);
            ASSERT_EQ(exec_result.error().errcode(), SQLITE_ABORT);
        } else {
            ASSERT_EQ(counter, 3);
            ASSERT_TRUE(exec_result);
        }
    }
}

TEST(database, exec_changes)
{
    auto db = sqlite::open(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE).value();
    ASSERT_TRUE(db.exec("CREATE TABLE foo (a INTEGER)"));

    ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(2)"));
    ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(2)"));
    ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(22)"));
    ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(22)"));
    ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(22)"));
    ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(222)"));
    ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(222)"));
    ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(222)"));
    ASSERT_TRUE(db.exec("INSERT INTO foo(a) VALUES(222)"));

    ASSERT_EQ(db.exec_changes("DELETE FROM foo WHERE a = 22"), 3);
}
