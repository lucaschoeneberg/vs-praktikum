//
// Created by Luca Schöneberg on 08.04.23.
//

#ifndef CLIENT_CLIENT_H
#define CLIENT_CLIENT_H

#include <string>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>

class Client {
public:
    Client(const std::string& server_address, int port);
    ~Client();

    void fetch_file(const std::string& file_name, int chunk_size, const std::string& output_file_name);

private:
    int sockfd;
    sockaddr_in server_addr;
    socklen_t server_len;
    std::vector<char> buffer;

    void request_file(const std::string& file_name, int chunk_size);
    void save_file(const std::string& output_file_name, int session_key);
};


#endif //CLIENT_CLIENT_H
