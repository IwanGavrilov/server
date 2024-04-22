#include "database.hpp"

DataBase * DataBase::p_database = nullptr;

DataBase::DataBase(std::string path_file) :s_path_file(path_file){
    
}

DataBase * DataBase::get_instance(const std::string path_file) {
    if (p_database == nullptr) {
        p_database = new DataBase(path_file);
    }
    return p_database;
}

// соединяемся с БД
int DataBase::connect() {
    n_code_connect = sqlite3_open(s_path_file.c_str(), &db);
    int result = 0;
    if (n_code_connect != SQLITE_OK) {
        result = n_code_connect;
    }
    return result;
}

void DataBase::disconnect() {
    sqlite3_close(db);
}

int DataBase::sql_exec(std::string s_sql_query) {
    n_cnt_result = 0;
    s_result_sql = "";
    kortegs.clear();
    
    // если код подключения не равен SQLITE_OK
    if (n_code_connect != SQLITE_OK) {
        return n_code_connect;
    }
    char * err_message = 0;
    const char * sql = s_sql_query.c_str();

    n_code_query = sqlite3_exec(db, sql, callback, 0, &err_message);
    if (n_code_query != SQLITE_OK) {
        s_err = std::string(err_message);
    }
    return n_code_query;    
}

int DataBase::sql_exec_select(std::string s_sql_query, std::vector< std::map<std::string, std::any> > & result_sql) {
    int result = 0;
    result = sql_exec(s_sql_query);
    if (result == SQLITE_OK) {
        result_sql = kortegs;
        return SQLITE_OK;
    }
    return result;
}

// выполняю запрос и вызвращаю результат в строку и кол-во данных
int DataBase::sql_exec_select(std::string s_sql_query, std::string & s_value_sql, int & n_count_value) {
   int result = 0;
   result = sql_exec(s_sql_query);
   if (result == SQLITE_OK) {
        s_value_sql = s_result_sql;
        n_count_value = n_cnt_result;
   }
   return result;
}

std::string DataBase::get_error() {
    std::string s_code = std::to_string(n_code_query);
    s_code += " " + s_err;
    return s_code;
}

DataBase::~DataBase() {
    sqlite3_close(db);
}

// возвращаем результат запроса
int DataBase::callback(void *notUsed, int colCount, char **columns, char **colNames) {
    std::map <std::string, std::any> fields;
    std::string s_fields;
    std::string s_value;
    std::string s_result {"{"};
    int n_idx = 0;
    for (int i = 0; i < colCount; i++)
    {
        s_fields = std::string(colNames[i]);
        s_value = columns[i] ? columns[i] : "NULL";
        if (n_idx == 0) {
            s_result += "\"" + s_fields + "\":\"" + s_value + "\"";
            n_idx++;
        }
        else {
            s_result += ",\"" + s_fields + "\":\"" + s_value + "\"";
        }
        fields[s_fields] = s_value;
        n_cnt_result++;                         // если запрос вовзращает результат, то увеличиваем счетчик
        /*
        std::cout << colNames[i] << " = " << columns[i] ? columns[i] : "NULL";
        std::cout << " ";
        */
    };
    s_result += "}";
    s_result_sql += s_result;
    kortegs.push_back(fields);
    //std::cout << "\n";
    return 0;
}