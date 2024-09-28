#include "sqlitecpp-thin/sqlite3-exception.hpp"
#include <cstdlib>

int main()
{
    auto db = sqlite::open(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    db.close();
    return EXIT_SUCCESS;
}
