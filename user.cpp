#include <vector>
#include <string>

class user
{
private:
    // имя
    std::string s_name {""};
    // логин
    std::string s_login {""};

    // список каталогов с ролью разработчик
    //std::vector<std::string> catalog_develop;

    // список каталогов с ролью просмотр
    //std::vector<std::string> catalog_viewer;
public:
    user(std::string name, std::string login) {
        s_name = name;
        s_login = login;
    };

    // добавляю каталог
    void add_catalog(std::string path, std::string role) {
        if (role == "r_develop") {
            //catalog_develop.push_back(path);
        }
        if (role == "r_viewer") {
            //catalog_viewer.push_back(path);
        }
    };

    // возвращаю каталоги с ролью разработчик
    std::vector<std::string>get_catalog_developer() {
        std::vector<std::string> catalog_develop;
        return catalog_develop;
    };
    
    // возвращаю каталоги с ролью просмотр
    std::vector<std::string>get_catalog_viewer() {
        std::vector<std::string> catalog_viewer;
        return catalog_viewer;
    };

    ~user();
};
