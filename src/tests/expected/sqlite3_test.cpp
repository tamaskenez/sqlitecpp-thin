#include "test_util.hpp"

namespace fs = std::filesystem;

#define UNICODE_STRING "test_unicode_ファイル名_𠜎𡃁💻💡.sql"
#define U8_UNICODE_STRING u8"test_unicode_ファイル名_𠜎𡃁💻💡.sql"

const char k_unicode_string[] = {char(0x74), char(0x65), char(0x73), char(0x74), char(0x5f), char(0x75), char(0x6e),
                                 char(0x69), char(0x63), char(0x6f), char(0x64), char(0x65), char(0x5f), char(0xe3),
                                 char(0x83), char(0x95), char(0xe3), char(0x82), char(0xa1), char(0xe3), char(0x82),
                                 char(0xa4), char(0xe3), char(0x83), char(0xab), char(0xe5), char(0x90), char(0x8d),
                                 char(0x5f), char(0xf0), char(0xa0), char(0x9c), char(0x8e), char(0xf0), char(0xa1),
                                 char(0x83), char(0x81), char(0xf0), char(0x9f), char(0x92), char(0xbb), char(0xf0),
                                 char(0x9f), char(0x92), char(0xa1), char(0x2e), char(0x73), char(0x71), char(0x6c),
                                 char(0)};

const char8_t k_u8_unicode_string[] = {char8_t(0x74), char8_t(0x65), char8_t(0x73), char8_t(0x74), char8_t(0x5f),
                                       char8_t(0x75), char8_t(0x6e), char8_t(0x69), char8_t(0x63), char8_t(0x6f),
                                       char8_t(0x64), char8_t(0x65), char8_t(0x5f), char8_t(0xe3), char8_t(0x83),
                                       char8_t(0x95), char8_t(0xe3), char8_t(0x82), char8_t(0xa1), char8_t(0xe3),
                                       char8_t(0x82), char8_t(0xa4), char8_t(0xe3), char8_t(0x83), char8_t(0xab),
                                       char8_t(0xe5), char8_t(0x90), char8_t(0x8d), char8_t(0x5f), char8_t(0xf0),
                                       char8_t(0xa0), char8_t(0x9c), char8_t(0x8e), char8_t(0xf0), char8_t(0xa1),
                                       char8_t(0x83), char8_t(0x81), char8_t(0xf0), char8_t(0x9f), char8_t(0x92),
                                       char8_t(0xbb), char8_t(0xf0), char8_t(0x9f), char8_t(0x92), char8_t(0xa1),
                                       char8_t(0x2e), char8_t(0x73), char8_t(0x71), char8_t(0x6c), char8_t(0)};

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
    print_string_bytes("                            UTF", k_unicode_string);
    print_string_bytes("                         U8_UTF", k_u8_unicode_string);
    fs::path p(k_unicode_string);
    fs::path p8(k_u8_unicode_string);
    print_string_bytes("     path(UTF).string().c_str()", p.string().c_str());
    print_string_bytes("  path(U8_UTF).string().c_str()", p8.string().c_str());
    print_string_bytes("   path(UTF).u8string().c_str()", p.u8string().c_str());
    print_string_bytes("path(U8_UTF).u8string().c_str()", p8.u8string().c_str());
    printf("raw u8 dump: ");
    int count = 0;
    for (auto* pp = k_u8_unicode_string;; ++pp) {
        auto c = static_cast<unsigned char>(*pp);
        printf(" %x", c);
        if (c == 0) {
            break;
        }
        ++count;
    }
    printf("\ncount: %d\n", count);
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
