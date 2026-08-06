#include "../src/sqlitecpp-thin/include/sqlitecpp-thin/sqlite3.hpp"

#include <cstdlib>

int main()
{
    auto db = sqlite::open(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    if (!db) {
        return EXIT_FAILURE;
    }
    auto closed = db->close();
    if (!closed) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
