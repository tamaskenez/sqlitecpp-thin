TEST(expected_error, failing_nonvoid_function_with_error)
{
    for (bool fail : {false, true}) {
        auto db =
          sqlite::open(fail ? "/nosuch/file/or/directory" : ":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
        ASSERT_EQ(db.has_value(), !fail);
        if (!db) {
            sqlite3* pdb{};
            auto errcode = sqlite3_open("/nosuch/file/or/directory", &pdb);
            EXPECT_EQ(errcode, sqlite3_errcode(pdb));
            auto e = db.error();
            EXPECT_EQ(e.errcode, sqlite3_errcode(pdb));
            EXPECT_EQ(e.extended_errcode, sqlite3_extended_errcode(pdb));
            EXPECT_EQ(e.errmsg, sqlite3_errmsg(pdb));
            EXPECT_EQ(e.error_offset, sqlite3_error_offset(pdb));
        }
    }
}

TEST(expected_error, failing_nonvoid_function_with_current_error)
{
    const char* badSql = "SELECT * FROM foo FROM";
    for (bool fail : {false, true}) {
        auto db = sqlite::open(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE).value();
        ASSERT_TRUE(db.exec("CREATE TABLE IF NOT EXISTS foo (a INTEGER)"));
        auto stmt = db.prepare(fail ? badSql : "SELECT * FROM foo");
        ASSERT_EQ(stmt.has_value(), !fail);
        if (!stmt) {
            auto* pdb = db.handle();
            sqlite3_stmt* pStmt;
            int rc = sqlite3_prepare_v2(pdb, badSql, -1, &pStmt, nullptr);
            auto ce = stmt.error();
            EXPECT_EQ(ce.errcode(), rc);
            auto e = ce.get_error();
            EXPECT_EQ(rc, sqlite3_errcode(pdb));
            EXPECT_EQ(e.errcode, sqlite3_errcode(pdb));
            EXPECT_EQ(e.extended_errcode, sqlite3_extended_errcode(pdb));
            EXPECT_EQ(e.errmsg, sqlite3_errmsg(pdb));
            EXPECT_EQ(e.error_offset, sqlite3_error_offset(pdb));

            EXPECT_EQ(db.errcode(), sqlite3_errcode(pdb));
            EXPECT_EQ(db.extended_errcode(), sqlite3_extended_errcode(pdb));
            EXPECT_EQ(db.errmsg(), sqlite3_errmsg(pdb));
            EXPECT_EQ(db.error_offset(), sqlite3_error_offset(pdb));
        }
    }
}

TEST(expected_error, failing_void_function_with_current_error)
{
    const char* badSql = "CREATE TABLE IF NOT EXISTS foo a INTEGER";
    for (bool fail : {false, true}) {
        auto db = sqlite::open(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE).value();
        auto execResult = db.exec(fail ? badSql : "CREATE TABLE IF NOT EXISTS foo (a INTEGER)");
        ASSERT_EQ(execResult.has_value(), !fail);
        if (!execResult) {
            int rc = sqlite3_exec(db.handle(), badSql, nullptr, nullptr, nullptr);
            auto ce = execResult.error();
            EXPECT_EQ(ce.errcode(), rc);
            auto e = ce.get_error();
            auto* pdb = db.handle();
            EXPECT_EQ(rc, sqlite3_errcode(pdb));
            EXPECT_EQ(e.errcode, sqlite3_errcode(pdb));
            EXPECT_EQ(e.extended_errcode, sqlite3_extended_errcode(pdb));
            EXPECT_EQ(e.errmsg, sqlite3_errmsg(pdb));
            EXPECT_EQ(e.error_offset, sqlite3_error_offset(pdb));
        }
    }
}
