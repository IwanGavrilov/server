#include <iostream>
#include <any>

//#include "server.hpp"
/*#include "server_async.hpp"

#include "database.hpp"

boost::asio::ip::tcp::acceptor acceptor(service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 15001));

void handle_accept(boost::shared_ptr<TCPServer> client, const boost::system::error_code & err __attribute__((unused))) {
    client->start();
    boost::shared_ptr<TCPServer> new_client = TCPServer::new_();
    acceptor.async_accept(new_client->sock(), boost::bind(handle_accept, new_client, _1));
}*/

/*std::vector<std::string> get_list_files(std::string s_path) {
    std::vector<std::string> list_file;
    for (const auto & entry : std::filesystem::directory_iterator(s_path)) {
        if (entry.is_regular_file()) {
            list_file.push_back(entry.path().filename().c_str());
        }
    }
    return list_file;
}*/
/*
int main() {
    boost::shared_ptr<TCPServer> client = TCPServer::new_();
    acceptor.async_accept(client->sock(), boost::bind(handle_accept, client, _1));
    service.run();
    return 0;
}
*/
