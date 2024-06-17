#include <string>

#include <include/json/json.h>
#include <fstream>

class config {
private:
    std::string s_path_database;
    std::string s_path_file;

public:
    config();

    std::string get_path_databse();
    std::string get_path_file();
};