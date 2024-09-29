#include "sqlite3.hpp"

#include "common.hpp"

#include <cassert>
#include <utility>
#include <variant>

namespace sqlite
{

namespace
{
template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template<typename... Ts, typename Variant>
auto switch_variant(Variant&& variant, Ts&&... ts)
{
    return std::visit(overloaded{std::forward<Ts>(ts)...}, std::forward<Variant>(variant));
}

error create_error_for_db(sqlite3* db)
{
    return error{
      .errcode = sqlite3_errcode(db),
      .extended_errcode = sqlite3_extended_errcode(db),
      .errmsg = sqlite3_errmsg(db),
      .error_offset = sqlite3_error_offset(db)
    };
}
} // namespace

#if SQLITECPPTHIN_EXCEPTION

exception::exception(error e)
    : _error(std::move(e))
    , _what(_error.format())
{
}

exception::exception(current_error e)
    : _error(e.get_error())
    , _what(_error.format())
{
}

#endif

expected<database, error> open(const char* filename, int flags)
{
    sqlite3* db{};
    int rc = sqlite3_open_v2(filename, &db, flags, nullptr);
    if (rc) {
        auto e = create_error_for_db(db);
        sqlite3_close_v2(db);
        RETURN_UNEXPECTED(MOVE(e));
    }
    return database(db);
}

expected<database, error> open(const string& filename, int flags)
{
    return open(filename.c_str(), flags);
}

expected<database, error> open(const fs::path& filename, int flags)
{
    auto u8string = filename.u8string();
    return open(reinterpret_cast<const char*>(u8string.c_str()), flags);
}

const char* errcode_macro_name(int rc)
{
#define CASE(X) \
    case X:     \
        return #X;
    switch (rc) {
        CASE(SQLITE_OK)
        CASE(SQLITE_ERROR)
        CASE(SQLITE_INTERNAL)
        CASE(SQLITE_PERM)
        CASE(SQLITE_ABORT)
        CASE(SQLITE_BUSY)
        CASE(SQLITE_LOCKED)
        CASE(SQLITE_NOMEM)
        CASE(SQLITE_READONLY)
        CASE(SQLITE_INTERRUPT)
        CASE(SQLITE_IOERR)
        CASE(SQLITE_CORRUPT)
        CASE(SQLITE_NOTFOUND)
        CASE(SQLITE_FULL)
        CASE(SQLITE_CANTOPEN)
        CASE(SQLITE_PROTOCOL)
        CASE(SQLITE_EMPTY)
        CASE(SQLITE_SCHEMA)
        CASE(SQLITE_TOOBIG)
        CASE(SQLITE_CONSTRAINT)
        CASE(SQLITE_MISMATCH)
        CASE(SQLITE_MISUSE)
        CASE(SQLITE_NOLFS)
        CASE(SQLITE_AUTH)
        CASE(SQLITE_FORMAT)
        CASE(SQLITE_RANGE)
        CASE(SQLITE_NOTADB)
        CASE(SQLITE_NOTICE)
        CASE(SQLITE_WARNING)
        CASE(SQLITE_ROW)
        CASE(SQLITE_DONE)
        CASE(SQLITE_ERROR_MISSING_COLLSEQ)
        CASE(SQLITE_ERROR_RETRY)
        CASE(SQLITE_ERROR_SNAPSHOT)
        CASE(SQLITE_IOERR_READ)
        CASE(SQLITE_IOERR_SHORT_READ)
        CASE(SQLITE_IOERR_WRITE)
        CASE(SQLITE_IOERR_FSYNC)
        CASE(SQLITE_IOERR_DIR_FSYNC)
        CASE(SQLITE_IOERR_TRUNCATE)
        CASE(SQLITE_IOERR_FSTAT)
        CASE(SQLITE_IOERR_UNLOCK)
        CASE(SQLITE_IOERR_RDLOCK)
        CASE(SQLITE_IOERR_DELETE)
        CASE(SQLITE_IOERR_BLOCKED)
        CASE(SQLITE_IOERR_NOMEM)
        CASE(SQLITE_IOERR_ACCESS)
        CASE(SQLITE_IOERR_CHECKRESERVEDLOCK)
        CASE(SQLITE_IOERR_LOCK)
        CASE(SQLITE_IOERR_CLOSE)
        CASE(SQLITE_IOERR_DIR_CLOSE)
        CASE(SQLITE_IOERR_SHMOPEN)
        CASE(SQLITE_IOERR_SHMSIZE)
        CASE(SQLITE_IOERR_SHMLOCK)
        CASE(SQLITE_IOERR_SHMMAP)
        CASE(SQLITE_IOERR_SEEK)
        CASE(SQLITE_IOERR_DELETE_NOENT)
        CASE(SQLITE_IOERR_MMAP)
        CASE(SQLITE_IOERR_GETTEMPPATH)
        CASE(SQLITE_IOERR_CONVPATH)
        CASE(SQLITE_IOERR_VNODE)
        CASE(SQLITE_IOERR_AUTH)
        CASE(SQLITE_IOERR_BEGIN_ATOMIC)
        CASE(SQLITE_IOERR_COMMIT_ATOMIC)
        CASE(SQLITE_IOERR_ROLLBACK_ATOMIC)
        CASE(SQLITE_IOERR_DATA)
        CASE(SQLITE_IOERR_CORRUPTFS)
        CASE(SQLITE_IOERR_IN_PAGE)
        CASE(SQLITE_LOCKED_SHAREDCACHE)
        CASE(SQLITE_LOCKED_VTAB)
        CASE(SQLITE_BUSY_RECOVERY)
        CASE(SQLITE_BUSY_SNAPSHOT)
        CASE(SQLITE_BUSY_TIMEOUT)
        CASE(SQLITE_CANTOPEN_NOTEMPDIR)
        CASE(SQLITE_CANTOPEN_ISDIR)
        CASE(SQLITE_CANTOPEN_FULLPATH)
        CASE(SQLITE_CANTOPEN_CONVPATH)
        CASE(SQLITE_CANTOPEN_DIRTYWAL)
        CASE(SQLITE_CANTOPEN_SYMLINK)
        CASE(SQLITE_CORRUPT_VTAB)
        CASE(SQLITE_CORRUPT_SEQUENCE)
        CASE(SQLITE_CORRUPT_INDEX)
        CASE(SQLITE_READONLY_RECOVERY)
        CASE(SQLITE_READONLY_CANTLOCK)
        CASE(SQLITE_READONLY_ROLLBACK)
        CASE(SQLITE_READONLY_DBMOVED)
        CASE(SQLITE_READONLY_CANTINIT)
        CASE(SQLITE_READONLY_DIRECTORY)
        CASE(SQLITE_ABORT_ROLLBACK)
        CASE(SQLITE_CONSTRAINT_CHECK)
        CASE(SQLITE_CONSTRAINT_COMMITHOOK)
        CASE(SQLITE_CONSTRAINT_FOREIGNKEY)
        CASE(SQLITE_CONSTRAINT_FUNCTION)
        CASE(SQLITE_CONSTRAINT_NOTNULL)
        CASE(SQLITE_CONSTRAINT_PRIMARYKEY)
        CASE(SQLITE_CONSTRAINT_TRIGGER)
        CASE(SQLITE_CONSTRAINT_UNIQUE)
        CASE(SQLITE_CONSTRAINT_VTAB)
        CASE(SQLITE_CONSTRAINT_ROWID)
        CASE(SQLITE_CONSTRAINT_PINNED)
        CASE(SQLITE_CONSTRAINT_DATATYPE)
        CASE(SQLITE_NOTICE_RECOVER_WAL)
        CASE(SQLITE_NOTICE_RECOVER_ROLLBACK)
        CASE(SQLITE_NOTICE_RBU)
        CASE(SQLITE_WARNING_AUTOINDEX)
        CASE(SQLITE_AUTH_USER)
        CASE(SQLITE_OK_LOAD_PERMANENTLY)
        CASE(SQLITE_OK_SYMLINK)
#undef CASE
    default:
        return nullptr;
    }
}

} // namespace sqlite
