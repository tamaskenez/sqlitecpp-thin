// This example writes a database of 3 integer columns, filled with random 4-byte integers.
//
// The first time it uses `bind_int()`, then the whole experiment is repeated with using `bind_int64()` (note that all
// integers still fit into 4-bytes). The file sizes of the resulting databases are printed to the console.
//
// The experiment verifies that using `bind_int()` or `bind_int64()` has no effect on the actual storage, thus providing
// a single, overloaded `bind_int` function on the C++ API is sufficient.

#include "sqlitecpp-thin/sqlite3-exception.hpp"

#include <array>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <random>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

constexpr std::string_view k_temp_dir_subdir = "sqlitecpp-thin-examples";

using AI3 = std::array<int, 3>;

namespace
{
size_t int_storage_test(const std::vector<AI3>& data, bool use_int64)
{
    auto system_temp_dir = fs::temp_directory_path();
    fs::create_directory(system_temp_dir / k_temp_dir_subdir);

    auto temp_file_path =
      system_temp_dir / std::string(k_temp_dir_subdir) / (std::to_string(time(nullptr)) + std::string(".db"));

    struct delete_on_exit_t {
        fs::path temp_file_path;
        ~delete_on_exit_t()
        {
            std::error_code ec;
            fs::remove(temp_file_path, ec);
        }
    } delete_on_exit{.temp_file_path = temp_file_path};

    {
        auto db = sqlite::open(temp_file_path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);

        db.exec(R"#(
            CREATE TABLE IF NOT EXISTS integers (
                a INTEGER PRIMARY KEY,
                b INTEGER NOT NULL,
                c INTEGER
            )
            )#");

        {
            auto stmt = db.prepare("INSERT INTO integers(a, b, c) VALUES(?, ?, ?)");
            for (auto& r : data) {
                for (size_t i = 0; i < 3; ++i) {
                    if (use_int64) {
                        stmt.bind_int(int(i + 1), int64_t(r[i]));
                    } else {
                        stmt.bind_int(int(i + 1), r[i]);
                    }
                }
                assert(stmt.step_done_changes() == 1);
                stmt.reset();
            }
        }

        // Verify number of rows.
        {
            auto stmt = db.prepare("SELECT count(1) FROM integers");
            switch (stmt.step()) {
            case sqlite::step_result::row:
                assert(std::cmp_equal(data.size(), stmt.column_int64(0)));
                break;
            case sqlite::step_result::done:
                assert(false);
                break;
            }
        }

        // Verify all numbers.
        {
            struct BC {
                int b, c;
            };
            std::unordered_map<int, BC> map;
            for (auto& r : data) {
                [[maybe_unused]] auto itb = map.insert(std::pair(r[0], BC{.b = r[1], .c = r[2]}));
                assert(itb.second);
            }
            auto stmt = db.prepare("SELECT a, b, c FROM integers");
            while (stmt.step() == sqlite::step_result::row) {
                auto a = stmt.column_int(0);
                [[maybe_unused]] auto b = stmt.column_int(1);
                [[maybe_unused]] auto c = stmt.column_int(2);
                auto it = map.find(a);
                if (it != map.end()) {
                    assert(it->second.b == b);
                    assert(it->second.c == c);
                } else {
                    assert(false);
                }
            }
        }
    }

    return fs::file_size(temp_file_path);
}
} // namespace

int main()
{
    try {
        std::default_random_engine dre;
        std::bernoulli_distribution bd;
        std::uniform_real_distribution<double> urd(0, log2(INT_MAX));

        constexpr size_t N = 8000;
        std::vector<AI3> data;
        data.reserve(N);

        // Random int, -INT_MAX..INT_MAX, with logarithmic distribution.
        auto random_int = [&]() {
            auto d = round(pow(2.0, urd(dre)));
            assert(0 <= d && d <= INT_MAX);
            if (bd(dre)) {
                d = -d;
            }
            return int(d);
        };

        std::unordered_set<int> primary_keys;

        for (size_t i = 0; i < N; ++i) {
            int primary_key;
            do {
                primary_key = random_int();
            } while (primary_keys.contains(primary_key));
            primary_keys.insert(primary_key);

            data.push_back(AI3{primary_key, random_int(), random_int()});
        }

        auto s1 = int_storage_test(data, false);
        auto s2 = int_storage_test(data, true);

        std::cout << "Database size using `bind_int()`: " << s1 << "\n";
        std::cout << "Database size using `bind_int64()`: " << s2 << "\n";

        return EXIT_SUCCESS;
    } catch (sqlite::exception& e) {
        std::cerr << "sqlite::exception: " << e.what() << "\n";
        return EXIT_FAILURE;
    } catch (std::exception& e) {
        std::cerr << "std::exception: " << e.what() << "\n";
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown exception.\n";
        return EXIT_FAILURE;
    }
}
