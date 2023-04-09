// Created by Luca Schöneberg on 07.04.23.
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#ifndef AB03_SERVER_H
#define AB03_SERVER_H

struct Session {
    bool is_active;

    int chunk_size;
    std::string file_name;
    int current_chunk;
    std::ifstream file_stream;
};

class Server {
public:
    Server();

    ~Server();

    [[maybe_unused]] void run();

    [[noreturn]] int init();

private:

    int init_session(int chunk_size, const std::string &file_name);

    int send_sid(int session_key, const sockaddr_in &client_addr);

    int send_data(int session_key, int chunk_no, const sockaddr_in &client_addr);

    int send_error(const std::string &reason, const sockaddr_in &client_addr);

    int receive_request(std::string &request, sockaddr_in &client_addr);

    void handle_request(const std::vector<char> &request, sockaddr_in &client_addr, socklen_t i);

    int sockfd;
    sockaddr_in server_addr;
    const int PORT = 8999;
    std::unordered_map<int, Session> sessions;
    int session_counter;

    static int read_file(Session &session, char *buffer, int chunk_size);

    int send_response(int chunk_no, int bytes_read, char *buffer, sockaddr_in &client_addr, socklen_t client_len) const;
};

#endif //AB03_SERVER_H
