#include "common.hpp"

namespace sqlite
{

error create_error_for_db(sqlite3* db)
{
    return error{
      .errcode = sqlite3_errcode(db),
      .extended_errcode = sqlite3_extended_errcode(db),
      .errmsg = sqlite3_errmsg(db),
      .error_offset = sqlite3_error_offset(db)
    };
}

} // namespace sqlite
