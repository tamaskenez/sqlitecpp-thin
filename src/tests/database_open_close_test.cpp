TEST(open, memory_const_char)
{
    auto db = sqlite::open(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    ASSERT_TRUE(db);
    ASSERT_TRUE(db->close());
}

TEST(open, memory_string)
{
    auto db = sqlite::open(std::string(":memory:"), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    ASSERT_TRUE(db);
    ASSERT_TRUE(db->close());
}

TEST(open, memory_path)
{
    auto db = sqlite::open(std::filesystem::path(":memory:"), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    ASSERT_TRUE(db);
    ASSERT_TRUE(db->close());
}

TEST(open, fail_on_invalid_file)
{
    auto db = sqlite::open("/nosuch/file/or/directory", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    ASSERT_FALSE(db);
}

TEST(open, moves)
{
    // Open 2 databases and remember raw handles.
    auto db1 = sqlite::open(std::filesystem::path(":memory:"), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    ASSERT_TRUE(db1);
    auto* db1_handle = db1->handle();
    auto db2 = sqlite::open(std::filesystem::path(":memory:"), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
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

    ASSERT_TRUE(db2->close());
    ASSERT_FALSE(db2->handle());

    // db2 was destructed here: `db2 = std::move(db3);`, close must fail.
    // Normally we get SQLITE_MISUSE from sqlite3_close. On Windows we get this:
    //
    //    unknown file: error: SEH exception with code 0xc0000005 thrown in the test body.
    //
    // That's why we skip this test on Windows.
    //
#ifdef _WIN32
    (void)db2_handle;
#else
    ASSERT_EQ(sqlite3_close(db2_handle), SQLITE_MISUSE);
#endif
}

TEST(open, database_destructor)
{
    sqlite3* db_handle{};
    {
        auto db = sqlite::open(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
        ASSERT_TRUE(db);
        db_handle = db->handle();
        // db destructed.
    }
    ASSERT_TRUE(db_handle);
    // close must fail.
    ASSERT_EQ(sqlite3_close(db_handle), SQLITE_MISUSE);
}
