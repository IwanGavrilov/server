#pragma once

#include "sqlite/sqlite3.h"
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <any>

class DataBase
{
private:
    // путь к файлу БД
    std::string s_path_file {""};
    // объект БД
    sqlite3 * db;
    // текст ошибки
    std::string s_err {""};
    // код ошибки соединения с БД
    int n_code_connect {0};
    // код ошибки выполнения запроса
    int n_code_query {0};

protected:
    // указатель на сам объект
    static DataBase * p_database;

    DataBase(std::string path_file);

    // возвращаемый массив кортежей
    static inline std::vector< std::map<std::string, std::any> > kortegs;
    
    /*
    возвращаемое значение sql-запроса. строка формата json
    [
        {<имя поля>:<значение>, <имя поля>:<значение> ... <имя поля>:<значение>}
        {<имя поля>:<значение>, <имя поля>:<значение> ... <имя поля>:<значение>}
        .............
        {<имя поля>:<значение>, <имя поля>:<значение> ... <имя поля>:<значение>}
    ]
    */
   static inline std::string s_result_sql {""};
   static inline int n_cnt_result {0};

    static int callback(void *notUsed, int colCount, char **columns, char **colNames);

    // выполнение запроса
    int sql_exec(std::string s_sql_query);

public:
    // запретил копирование
    void operator = (DataBase & db) = delete;
    //void operator () (std::string path_file) = delete;

    // создание экземпляра
    static DataBase * get_instance(const std::string path_file);

    // соединение с БД
    int connect();

    // выполняю запрос и вызвращаю результат в виде контейнера
    int sql_exec_select(std::string s_sql_query, std::vector< std::map<std::string, std::any> > & result_sql);

    // выполняю запрос и вызвращаю результат в строку и кол-во данных
    int sql_exec_select(std::string s_sql_query, std::string & s_value_sql, int & n_count_value);

    // выводим код и текст ошибки
    std::string get_error();

    // разрываем соединение
    void disconnect();

    ~DataBase();
};

