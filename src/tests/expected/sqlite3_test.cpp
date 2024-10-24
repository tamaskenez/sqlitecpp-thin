#include "test_util.hpp"

namespace fs = std::filesystem;

namespace
{
using uchar = unsigned char;

const char* k_unicode_string = "test_unicode_ファイル名_𠜎𡃁💻💡.sql";
const char8_t* k_u8_unicode_string = reinterpret_cast<const char8_t*>(k_unicode_string);

const char8_t k_u8_unicode_string_as_array[] = {
  char8_t(0x74), char8_t(0x65), char8_t(0x73), char8_t(0x74), char8_t(0x5f), char8_t(0x75), char8_t(0x6e),
  char8_t(0x69), char8_t(0x63), char8_t(0x6f), char8_t(0x64), char8_t(0x65), char8_t(0x5f), char8_t(0xe3),
  char8_t(0x83), char8_t(0x95), char8_t(0xe3), char8_t(0x82), char8_t(0xa1), char8_t(0xe3), char8_t(0x82),
  char8_t(0xa4), char8_t(0xe3), char8_t(0x83), char8_t(0xab), char8_t(0xe5), char8_t(0x90), char8_t(0x8d),
  char8_t(0x5f), char8_t(0xf0), char8_t(0xa0), char8_t(0x9c), char8_t(0x8e), char8_t(0xf0), char8_t(0xa1),
  char8_t(0x83), char8_t(0x81), char8_t(0xf0), char8_t(0x9f), char8_t(0x92), char8_t(0xbb), char8_t(0xf0),
  char8_t(0x9f), char8_t(0x92), char8_t(0xa1), char8_t(0x2e), char8_t(0x73), char8_t(0x71), char8_t(0x6c),
  char8_t(0)
};

const fs::path test_dir_name("a_test_dir");

class UnicodeFilenameHelper
{
    const fs::path pwd;
    const fs::path test_file_path;

public:
    UnicodeFilenameHelper()
        : pwd(fs::current_path())
        , test_file_path(pwd / test_dir_name / fs::path(k_u8_unicode_string))
    {
        fs::remove_all(pwd / test_dir_name);
        fs::create_directory(pwd / test_dir_name);
        fs::current_path(pwd / test_dir_name);
        CHECK(!fs::exists(test_file_path));
    }
    void test()
    {
        CHECK(fs::is_regular_file(test_file_path));
    }
    ~UnicodeFilenameHelper()
    {
        fs::current_path(pwd);
        fs::remove_all(pwd / test_dir_name);
    }
};

} // namespace

TEST(open, utf8_filename_strings_in_hex)
{
    // Verify strings.
    for (size_t i = 0;; ++i) {
        char8_t c = k_u8_unicode_string_as_array[i];
        char8_t c1 = char8_t(k_unicode_string[i]);
        char8_t c2 = k_u8_unicode_string[i];
        EXPECT_EQ(c, c1);
        EXPECT_EQ(c, c2);
        if (!c || !c1 || !c2) {
            break;
        }
    }
}

TEST(open, utf8_filename_ground_truth)
{
    UnicodeFilenameHelper ufh;

    sqlite3* pDb{};
    ASSERT_EQ(sqlite3_open_v2(k_unicode_string, &pDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr), SQLITE_OK);
    ASSERT_EQ(sqlite3_close(pDb), SQLITE_OK);
    ufh.test();
}

TEST(open, utf8_filename_fs_path)
{
    UnicodeFilenameHelper ufh;
    {
        ASSERT_TRUE(sqlite::open(fs::path(k_u8_unicode_string), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));
    }
    ufh.test();
}
