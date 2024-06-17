#include "config.hpp"

/*
std::ifstream people_file("people.json", std::ifstream::binary);
Json::Value people;
people_file >> people;

cout<<people; //This will print the entire json object.

//The following lines will let you access the indexed objects.
cout<<people["Anna"];
*/

config::config() {
    std::ifstream config_file("config.json", std::ifstream::binary);
    Json::Value config;
    config_file >> config;
    
}

std::string config::get_path_databse() {
    return s_path_database;
}

std::string config::get_path_file() {
    return s_path_file;
}