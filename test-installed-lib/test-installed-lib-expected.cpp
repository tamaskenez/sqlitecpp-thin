#include "sqlite3cpp-thin/sqlite3-expected.hpp"
#include <cstdlib>

int main()
{
    auto db = sqlite::open(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    if (!db) {
        return EXIT_FAILURE;
    }
    auto closed = db.close();
    if (!closed) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
