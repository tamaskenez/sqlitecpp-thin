#include "sqlitecpp-thin/sqlite3-exception.hpp"

#include <cstdlib>
#include <filesystem>
#include <ctime>

namespace fs = std::filesystem;

constexpr std::string_view k_temp_dir_subdir = "sqlitecpp-thin-examples";

size_t int_storage_test(bool use_int64){
    auto system_temp_dir = fs::temp_directory_path();
    fs::create_directory(system_temp_dir / k_temp_dir_subdir);
    
    auto temp_file_path = system_temp_dir / std::string(k_temp_dir_subdir) / (std::to_string(time(nullptr)) + std::string(".db"));
    
    struct delete_on_exit_t {
        fs::path temp_file_path;
        ~delete_on_exit_t(){
            std::error_code ec;
            fs::remove(temp_file_path, ec);
        }
    } delete_on_exit{.temp_file_path=temp_file_path};
    
    auto db = sqlite::open(temp_file_path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    
    db.
}

int main(){
    return EXIT_SUCCESS;
}
