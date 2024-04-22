#pragma once

/* 
#ifdef WIN32
#define _WIN32_WINNT 0x0501
#include <stdio.h>
#endif
*/

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/algorithm/string.hpp>

#include <functional>
#include <optional>
#include <unordered_set>
#include <queue>
#include <iostream>
#include <fstream>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <filesystem>

#include "database.hpp"

class TCPServer : public boost::enable_shared_from_this<TCPServer>, boost::noncopyable 
{
    typedef TCPServer self_type;

private:
    boost::asio::ip::tcp::socket socket_;
    enum { max_msg = 1024 };
    char read_buffer_[max_msg];
    char write_buffer_[max_msg];
    bool started_;
    std::string username_;
    std::string s_root_folder {"/media/waine-86/WORK/My_Project_C_Delphy/OTUS/Project/Sources/SrvTCP/"};
    boost::asio::deadline_timer timer_;
    boost::posix_time::ptime last_ping;
    bool clients_changed_;

    DataBase * db;
    //std::string s_root{"SrvTCP"};
    std::string s_new_file_folder {""};
    std::string s_new_file_name {""};
public:
    //typedef boost::shared_ptr<TCPServer> ptr;
    
    TCPServer();
    
    void start();

    static boost::shared_ptr<TCPServer> new_();
    //boost::shared_ptr<TCPServer> new_();

    void stop();

    bool started() const;
    boost::asio::ip::tcp::socket & sock();
    std::string get_username() const;
    void set_clients_changed();

private:
    void on_read(const boost::system::error_code & err, size_t bytes);
    void do_read();

    void on_write(const boost::system::error_code & err, size_t bytes);
    void do_write(const std::string & msg);
    
    void on_login(const std::string & msg);
    void on_catalogs(const std::string & msg);
    void on_add_roles(const std::string & msg);
    void on_del_roles(const std::string & msg);
    void on_add_catalog(const std::string & msg);

    void on_add_file(const std::string & msg);
    void on_save_file(const std::string & msg);
    
    // sql запросы. возвращаемое значение - json или bool
    bool query_execute_login(std::string s_login, std::string s_password);
    std::string query_execute_catalogs(std::string s_login);
    bool query_add_role(std::string s_login, std::string s_role, std::string s_path);
    bool query_del_role(std::string s_login, std::string s_role, std::string s_path);
    bool query_add_catalog(std::string s_login, std::string s_path);

    // соединяемся с БД
    bool connect_database();

    // возврат списка файлов из каталога
    std::vector<std::string> get_list_files(std::string s_path);

    size_t read_complete(const boost::system::error_code & err, size_t bytes);
};

inline boost::asio::io_service service;

typedef boost::shared_ptr<TCPServer> client_ptr;
typedef std::vector<client_ptr> array;

inline array clients;
