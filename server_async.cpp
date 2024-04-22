#include "server_async.hpp"


using namespace boost::asio;
using namespace boost::posix_time;


#define MEM_FN(x)       boost::bind(&self_type::x, shared_from_this())
#define MEM_FN1(x,y)    boost::bind(&self_type::x, shared_from_this(),y)
#define MEM_FN2(x,y,z)  boost::bind(&self_type::x, shared_from_this(),y,z)

void update_clients_changed();

TCPServer::TCPServer() : socket_(service), started_(false), timer_(service), clients_changed_(false) {
    db = DataBase::get_instance(
            "/media/waine-86/WORK/My_Project_C_Delphy/OTUS/Project/Sources/database/data_base.db"
        );

    // устанавливаем соединение с БД
    if (!connect_database()) {
        std::cout << "Ошибка подключения к \"БД\"" << std::endl;
    }
    else {
        std::string s = "БД";
        std::string s_o = "Соединение с \"" + s + "\" установлено";
        std::cout << s_o << std::endl;
        // т.к. БД не сетевая, то отключаемся.
        // проверил таким образом наличие БД
        db->disconnect();
    }
}

void TCPServer::start() {
    started_ = true;
    clients.push_back( shared_from_this());
    last_ping = boost::posix_time::microsec_clock::local_time();
    // first, we wait for client to login
    do_read();
}

boost::shared_ptr<TCPServer> TCPServer::new_() {
    boost::shared_ptr<TCPServer> new_(new TCPServer());
    return new_;
}

void TCPServer::stop() {
    if ( !started_) return;
    started_ = false;
    socket_.close();

    boost::shared_ptr<TCPServer> self = shared_from_this();
    array::iterator it = std::find(clients.begin(), clients.end(), self);
    clients.erase(it);
    std::cout << "client disconnect" << std::endl;
    update_clients_changed();
}

void TCPServer::on_read(const boost::system::error_code & err, size_t bytes) {
        if ( err) stop();
        if ( !started() ) return;

        std::string msg(read_buffer_, bytes);
        if (msg.find("connect ") == 0) {
            on_login(msg);
            return;
        }

        if (msg.find("catalogs ") == 0) {
            on_catalogs(msg);
            return;
        }

        if (msg.find("set_role ") == 0) {
            on_add_roles(msg);
            return;
        }

        if (msg.find("del_role ") == 0) {
            on_del_roles(msg);
            return;
        }

        if (msg.find("add_catalog ") == 0) {
            on_add_catalog(msg);
            return;
        }

        if (msg.find("add_file ") == 0) {
            on_add_file(msg);
            return;
        }
        on_save_file(msg);
        return;/*else on_answer(msg);*/
    }
    
void TCPServer::on_login(const std::string & msg) {
    std::istringstream in(msg);
    std::string s_data;
    in >> s_data >> s_data;
    // первый элемент вектора login, второй password
    std::vector<std::string> strs;

    boost::split(strs, s_data, boost::is_any_of("#"));
    username_ = strs[0];
    
    if (query_execute_login(username_, strs[1])) {
        std::cout << username_ << " logged in" << std::endl;
        do_write("login_ok\n");
        update_clients_changed();
    }
    else {
        do_write("login_no\n");
        stop();
    }
}

void TCPServer::on_catalogs(const std::string & msg) {
    std::istringstream in(msg);
    std::string s_data;
    in >> s_data >> s_data;

    if (username_ == s_data) {
        // получаю список каталогов с ролями для пользователя
        std::string s_out = query_execute_catalogs(username_);
        if (!s_out.empty()) {
            do_write("catalogs_ok " + s_out + "\n");
        }
        else {
            do_write("catalogs_no\n");
        }
    }   
}

void TCPServer::on_add_roles(const std::string & msg) {
    std::istringstream in(msg);
    std::string s_data;
    in >> s_data >> s_data;
    // первый элемент вектора login, второй role, третий path
    std::vector<std::string> strs;

    boost::split(strs, s_data, boost::is_any_of("#"));
    
    if (query_add_role(strs[0], strs[1], strs[2])) {
        std::cout << strs[0] << " role add " << strs[1] << std::endl;
        do_write("add_role_ok\n");
        update_clients_changed();
    }
    else {
        do_write("add_role_no\n");
        stop();
    }
}

void TCPServer::on_del_roles(const std::string & msg) {
    std::istringstream in(msg);
    std::string s_data;
    in >> s_data >> s_data;
    // первый элемент вектора login, второй role, третий path
    std::vector<std::string> strs;

    boost::split(strs, s_data, boost::is_any_of("#"));
    
    if (query_del_role(strs[0], strs[1], strs[2])) {
        std::cout << strs[0] << " del_role " << strs[1] << std::endl;
        do_write("del_role_ok\n");
        update_clients_changed();
    }
    else {
        do_write("del_role_no\n");
        stop();
    }
}

void TCPServer::on_add_catalog(const std::string & msg) {
    std::istringstream in(msg);
    std::string s_data;
    in >> s_data >> s_data;
    // первый элемент вектора login, второй role, третий path
    std::vector<std::string> strs;

    boost::split(strs, s_data, boost::is_any_of("#"));
    
    if (query_add_catalog(strs[0], strs[1])) {
        std::cout << strs[0] << " add catalog " << strs[1] << std::endl;
        do_write("add_catalog_ok\n");
        update_clients_changed();
    }
    else {
        do_write("add_catalog_no\n");
        stop();
    }
}

void TCPServer::on_add_file(const std::string & msg) {
    std::istringstream in(msg);
    std::string s_data;
    in >> s_data >> s_data;
    std::vector<std::string> strs;

    boost::split(strs, s_data, boost::is_any_of("#"));
    // каталог куда положить файл
    s_new_file_folder = strs[0];
    // имя файла
    s_new_file_name = strs[1];
    std::cout << "Ждем загрузки файла" << std::endl;
    do_write("add_file_expectation_on\n");
    //do_read_file();
}

// читаем и сохраняем файл на диск
void TCPServer::on_save_file(const std::string & msg) {
    // сохраняем данные в файл
    std::string s_full_path = s_root_folder + "/" + s_new_file_folder + "/" + s_new_file_name;
    std::ofstream out_file;
    out_file.open(s_full_path);
    
    if (!out_file.is_open()) {
        std::cout << "Файл не открыт" << std::endl;
        do_write("file_save_no\n");
        return;
    }
    out_file << msg;
    out_file.close();
    std::cout << "Файл сохранен : " << s_full_path << std::endl;
    do_write("file_save_ok\n");
    stop();
}

void TCPServer::on_write(const boost::system::error_code & err, size_t bytes) {
    do_read();
}

void TCPServer::do_read() {
    async_read(socket_, buffer(read_buffer_), 
                MEM_FN2(read_complete, _1, _2), MEM_FN2(on_read,_1,_2));
    //post_check_ping();
}

void TCPServer::do_write(const std::string & msg) {
    if ( !started() ) return;
    std::copy(msg.begin(), msg.end(), write_buffer_);
    socket_.async_write_some( buffer(write_buffer_, msg.size()), 
                            MEM_FN2(on_write,_1,_2));
}

size_t TCPServer::read_complete(const boost::system::error_code & err, size_t bytes) {
    if ( err) return 0;
    bool found = std::find(read_buffer_, read_buffer_ + bytes, '\n') < read_buffer_ + bytes;
    // we read one-by-one until we get to enter, no buffering
    return found ? 0 : 1;
}

bool TCPServer::started() const { 
    return started_;
}
ip::tcp::socket & TCPServer::sock() {
    return socket_;
}

std::string TCPServer::get_username() const {
    return username_; 
}

void TCPServer::set_clients_changed(){
    clients_changed_ = true; 
}

void update_clients_changed() {
    for( array::iterator b = clients.begin(), e = clients.end(); b != e; ++b)
        (*b)->set_clients_changed();
}

bool TCPServer::connect_database() {
    int result_sqlite = 0;
    result_sqlite = db->connect();
    if (result_sqlite != SQLITE_OK) {
        std::cout << "Ошибка подключения к БД" << std::endl;
        return false;
    }
    else {
        std::cout << "Код ошибки подключения : " << result_sqlite << std::endl;
    }
    return true;
}

bool TCPServer::query_execute_login(std::string s_login, std::string s_password) {
    std::cout << "Пользователь : " << s_login << ". Соединение с БД установлено." << std::endl;
    // соединение усановлено. выполняем запрос и отключаемся

    std::string s_sql = "select users.sys_id from users where users.s_login = '" 
            + s_login + "' AND users.s_pswrd = '" + s_password + "'";
    if (!connect_database()) {
        return false;
    }
    //std::vector< std::map<std::string, std::any> > korteg;
    //int result_sqlite = db->sql_exec_select(s_sql, korteg);
    std::string s_value {""};
    int n_count = 0;
    int result_sqlite = db->sql_exec_select(s_sql, s_value, n_count);
    if (result_sqlite != SQLITE_OK) {
        // ошибка при выполнении запроса
        std::cout << db->get_error() << std::endl;
        db->disconnect();
        return false;
    }
    if (n_count = 0) {
        // не найдены данные. авторизация не прошла
        db->disconnect();
        return false;
    }
    //std::map<std::string, std::any> data = korteg[0];
    //std::cout << username_ << " logged in. sys_id = " << std::any_cast<std::string>(data["sys_id"]) << std::endl;

    db->disconnect();
    return true;
}
/*
возвращаемое значение: строка в формате json
*/
std::string TCPServer::query_execute_catalogs(std::string s_login) {
    std::string s_out_value {""};
    int n_count {0};
    std::string s_sql = "select folders.s_path AS 'folder', roles.sys_id AS 'role' from main_table";
                s_sql += " LEFT join users";
                s_sql += " on main_table.sys_id_user = users.sys_id";
                s_sql += " LEFT join folders";
                s_sql += " on main_table.syd_id_folder = folders.sys_id";
                s_sql += " LEFT join roles";
                s_sql += " on main_table.sys_id_role = roles.sys_id";
                s_sql += " where main_table.sys_id_user in (";
                s_sql += " select users.sys_id from users";
                s_sql += " where users.s_login = '" + s_login + "')";
    if (!connect_database()) {
        // не соедился с БД
        return s_out_value;
    }
    // возвращаемый массив кортежей
    std::vector< std::map<std::string, std::any> > kortegs;
    int result = db->sql_exec_select(s_sql, kortegs);
    //int result = db->sql_exec_select(s_sql, s_out_value, n_count);
    if (result != SQLITE_OK) {
        // ошибка при выполнении запроса
        std::cout << db->get_error() << std::endl;
        db->disconnect();
        return s_out_value = "";
    }
    //if (n_count = 0) {
    if (kortegs.empty()) {
        // если нет данных, то на всякий случай очищаем строку
        db->disconnect();
        return s_out_value = "";
    }
    //std::map<std::string, std::any> data = korteg[0];
    //std::cout << username_ << " logged in. sys_id = " << std::any_cast<std::string>(data["sys_id"]) << std::endl;
    std::string s_res_tmp;
    int n_idx = 0;
    s_out_value = "[";
    for(auto korteg : kortegs) {
        if (s_out_value == "[") { s_out_value += "{"; } else { s_out_value += ",{"; }
        for(auto data : korteg) {
            if (n_idx == 0) {
                s_out_value += "\"" + data.first + "\":\"" + std::any_cast<std::string>(data.second) + "\"";
                n_idx++;
            } else {
                s_out_value += ",\"" + data.first + "\":\"" + std::any_cast<std::string>(data.second) + "\"";
            }
            if (data.first == "folder") {
                std::vector<std::string> lst_file = get_list_files(s_root_folder + "/" + std::any_cast<std::string>(data.second));
                if (!lst_file.empty()) {
                    s_res_tmp = ",\"files\":[";
                    int n_cnt = 0;
                    for(std::string s_file : lst_file) {
                        if (n_cnt == 0) {
                            s_res_tmp += "\"" + s_file + "\"";
                            n_cnt++;
                        }
                        else {
                            s_res_tmp += ",\"" + s_file + "\"";
                        }
                    }
                    s_res_tmp += "]";
                    s_out_value += s_res_tmp;
                }
            }
        }
        s_out_value += "}";
        n_idx = 0;
    }
    s_out_value += "]";
    db->disconnect();
    return s_out_value;
}

std::vector<std::string> TCPServer::get_list_files(std::string s_path) {
    std::vector<std::string> list_file;
    for (const auto & entry : std::filesystem::directory_iterator(s_path)) {
        if (entry.is_regular_file()) {
            list_file.push_back(entry.path().filename().c_str());
        }
    }
    return list_file;
}

bool TCPServer::query_add_role(std::string s_login, std::string s_role, std::string s_path) {
    std::string s_out_value {""};
    int n_count {0};
    std::string s_sql = "INSERT INTO main_table(syd_id_folder, sys_id_user, sys_id_role) ";
                s_sql += "VALUES (";
	            s_sql += "(SELECT folders.sys_id FROM folders WHERE folders.s_path = '" + s_path + "'),";
	            s_sql += "(SELECT users.sys_id FROM users WHERE users.s_login = '" + s_login + "'),";
	            s_sql += "(SELECT roles.sys_id FROM roles WHERE roles.description = '" + s_role + "'));";
    if (!connect_database()) {
        // не соедился с БД
        return false;
    }
    int result = db->sql_exec_select(s_sql, s_out_value, n_count);
    if (result != SQLITE_OK) {
        std::cout << db->get_error() << std::endl;
        db->disconnect();
        return false;
    }
    db->disconnect();
    return true;
}

bool TCPServer::query_add_catalog(std::string s_login, std::string s_path) {
    std::string s_out_value {""};
    int n_count {0};
    std::string s_sql = "INSERT INTO folders(s_path) values(\"" + s_path + "\"); ";
                s_sql += "INSERT INTO main_table(syd_id_folder, sys_id_user, sys_id_role) ";
                s_sql += "VALUES (";
	            s_sql += "(SELECT folders.sys_id FROM folders WHERE folders.s_path = 'bsaghlvgsahvhsafsa'),";
	            s_sql += "(SELECT users.sys_id FROM users WHERE users.s_login = '" + s_login + "'),";
	            s_sql += "(SELECT roles.sys_id FROM roles WHERE roles.description = 'Разработчик'));";
    if (!connect_database()) {
        // не соедился с БД
        return false;
    }
    int result = db->sql_exec_select(s_sql, s_out_value, n_count);
    if (result != SQLITE_OK) {
        std::cout << db->get_error() << std::endl;
        db->disconnect();
        return false;
    }
    db->disconnect();
    return true;
}

bool TCPServer::query_del_role(std::string s_login, std::string s_role, std::string s_path) {
    std::string s_out_value {""};
    int n_count {0};
    std::string s_sql = "DELETE FROM main_table ";
                s_sql += "WHERE main_table.syd_id_folder = (";
                s_sql += "SELECT folders.sys_id FROM folders WHERE folders.s_path LIKE '" + s_path + "') ";
                s_sql += "AND sys_id_role LIKE '" + s_role + "' ";
                s_sql += "AND main_table.sys_id_user = (";
                s_sql += "SELECT users.sys_id FROM users WHERE users.s_login LIKE '" + s_login + "')";   
    if (!connect_database()) {
        // не соедился с БД
        return false;
    }
    int result = db->sql_exec_select(s_sql, s_out_value, n_count);
    if (result != SQLITE_OK) {
        std::cout << db->get_error() << std::endl;
        db->disconnect();
        return false;
    }
    db->disconnect();
    return true;
}
