#include "test_util.hpp"

namespace fs = std::filesystem;

#define UNICODE_STRING "test_unicode_ファイル名_𠜎𡃁💻💡.sql"
#define U8_UNICODE_STRING u8"test_unicode_ファイル名_𠜎𡃁💻💡.sql"

const fs::path test_dir_name("a_test_dir");

namespace
{
using uchar = unsigned char;
void print_string_bytes(const char* title, const char* s)
{
    auto l = strlen(s);
    printf("%s,   size: %zu * char::", title, l);
    for (size_t i = 0; i < l; ++i) {
        printf(" %x", uchar(s[i]));
    }
    printf("\n");
}

void print_string_bytes(const char* title, const char8_t* s)
{
    auto l = strlen(reinterpret_cast<const char*>(s));
    printf("%s, size: %zu * char8_t:", title, l);
    for (size_t i = 0; i < l; ++i) {
        printf(" %x", uchar(s[i]));
    }
    printf("\n");
}

class UnicodeFilenameHelper
{
    const fs::path pwd;
    const fs::path test_file_path;

public:
    UnicodeFilenameHelper()
        : pwd(fs::current_path())
        , test_file_path(pwd / test_dir_name / fs::path(U8_UNICODE_STRING))
    {
        fs::remove_all(pwd / test_dir_name);
        fs::create_directory(pwd / test_dir_name);
        fs::current_path(pwd / test_dir_name);
        CHECK(!fs::exists(test_file_path));
    }
    void test()
    {
        auto parent = test_file_path.parent_path();
        std::cout << "parent: " << parent << "\n";
        for (auto& de : fs::directory_iterator(parent)) {
            print_string_bytes("  string", de.path().filename().string().c_str());
            print_string_bytes("u8string", de.path().filename().u8string().c_str());
            // std::cout << "entry: " << de.path().filename() << "\n";
        }
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
    print_string_bytes("                            UTF", UNICODE_STRING);
    print_string_bytes("                         U8_UTF", U8_UNICODE_STRING);
    fs::path p(UNICODE_STRING);
    fs::path p8(U8_UNICODE_STRING);
    print_string_bytes("     path(UTF).string().c_str()", p.string().c_str());
    print_string_bytes("  path(U8_UTF).string().c_str()", p8.string().c_str());
    print_string_bytes("   path(UTF).u8string().c_str()", p.u8string().c_str());
    print_string_bytes("path(U8_UTF).u8string().c_str()", p8.u8string().c_str());
}

TEST(open, utf8_filename_ground_truth)
{
    UnicodeFilenameHelper ufh;

    sqlite3* pDb{};
    ASSERT_EQ(sqlite3_open_v2(UNICODE_STRING, &pDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr), SQLITE_OK);
    ASSERT_EQ(sqlite3_close(pDb), SQLITE_OK);
    ufh.test();
}

TEST(open, utf8_filename_fs_path)
{
    UnicodeFilenameHelper ufh;
    {
        ASSERT_TRUE(sqlite::open(fs::path(U8_UNICODE_STRING), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));
    }
    ufh.test();
}
