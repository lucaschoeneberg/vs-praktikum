//
// Created by Luca Schöneberg on 08.04.23.
//

#include "client.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <arpa/inet.h>

const int BUFFER_SIZE = 65536;

/**
 * Initializes the client
 *
 * @param server_address
 * @param port
 */
Client::Client(const std::string &server_address, int port) : buffer(BUFFER_SIZE) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_address.c_str(), &server_addr.sin_addr);
    server_len = sizeof(server_addr);
}

Client::~Client() {
    close(sockfd);
}

/**
 * fetch_file fetches a file from the server
 *
 * @param file_name
 * @param chunk_size
 * @param output_file_name
 * @return
 */
void Client::fetch_file(const std::string &file_name, int chunk_size, const std::string &output_file_name) {
    std::cout << "Fetching file " << file_name << " with chunk size " << chunk_size << std::endl;
    request_file(file_name, chunk_size);

    init_buffer();

    int received = recvfrom(sockfd, buffer.data(), BUFFER_SIZE, 0, (sockaddr *) &server_addr, &server_len);

    if (received > 0) {
        buffer.resize(received);
        std::istringstream response_stream(std::string(buffer.begin(), buffer.end()));
        std::string response_type;
        getline(response_stream, response_type, ';');

        if (response_type == "HSOSSTP_SIDXX") {
            int session_key;
            response_stream >> session_key;
            save_file(output_file_name, session_key);
        }
    }
}

/**
 * request_file requests a file from the server
 *
 * @param file_name
 * @param chunk_size
 */
void Client::request_file(const std::string &file_name, int chunk_size) {
    std::ostringstream init_request;
    init_request << "HSOSSTP_INITX;" << chunk_size << ";" << file_name;

    std::string init_request_str = init_request.str();
    sendto(sockfd, init_request_str.c_str(), init_request_str.size(), 0, reinterpret_cast<sockaddr *>(&server_addr),
           server_len);
    std::cout << "Sending request: " << init_request_str << " done" << std::endl;
}

/**
 * save_file saves a file from the server
 *
 * @param output_file_name
 * @param session_key
 */
void Client::save_file(const std::string &output_file_name, int session_key) {
    std::ofstream output_file(output_file_name, std::ios::binary);
    if (!output_file.is_open()) {
        std::cout << "Could not open output file" << std::endl;
        return;
    }

    int chunk_no = 0;
    while (true) {
        std::string get_request = "HSOSSTP_GETXX;" + std::to_string(session_key) + ";" + std::to_string(chunk_no);
        sendto(sockfd, get_request.c_str(), get_request.size(), 0, reinterpret_cast<sockaddr *>(&server_addr),
               server_len);

        init_buffer();

        int received = static_cast<int>(recvfrom(sockfd, buffer.data(), BUFFER_SIZE, 0, (sockaddr *) &server_addr,
                                                 &server_len));
        if (received == -1) {
            std::cout << "Error receiving data: " << strerror(errno) << std::endl;
            break;
        }
        if (received == 0) {
            std::cout << "Connection closed" << std::endl;
            break;
        }

        buffer.resize(received);
        std::istringstream data_response(std::string(buffer.begin(), buffer.end()));
        std::string data_type;
        getline(data_response, data_type, ';');

        if (data_type == "HSOSSTP_DATAX") {
            int chunk_no_received, bytes_read;
            data_response >> chunk_no_received >> std::ws;
            std::string bytes_read_str;
            getline(data_response, bytes_read_str, ';');
            getline(data_response, bytes_read_str, ';');
            bytes_read = std::stoi(bytes_read_str);

            if (chunk_no_received != chunk_no) {
                std::cout << "Received wrong chunk" << std::endl;
                break;
            }

            std::vector<char> data_buffer(bytes_read);
            data_response.read(data_buffer.data(), bytes_read);
            output_file.write(data_buffer.data(), bytes_read);

            chunk_no++;
        } else {
            break;
        }
    }
    std::cout << "File saved" << std::endl;
    output_file.close();
}

/**
 * init_buffer initializes the buffer
 */
void Client::init_buffer() {
    std::fill(buffer.begin(), buffer.end(), 0);
    buffer.resize(BUFFER_SIZE);
}

