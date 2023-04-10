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

void Client::fetch_file(const std::string &file_name, int chunk_size, const std::string &output_file_name) {
    request_file(file_name, chunk_size);

    init_buffer();

    int received = recvfrom(sockfd, buffer.data(), BUFFER_SIZE, 0, (sockaddr *) &server_addr, &server_len);

    if (received > 0) {
        buffer.resize(received);
        std::string response(buffer.begin(), buffer.end());

        std::istringstream response_stream(response);
        std::string response_type(13, 0);
        std::cout << response_stream.str() << std::endl;
        int session_key;
        int response_chunk_count;

        sscanf(response.c_str(), "%[^;];%d;%d", response_type.data(), &session_key, &response_chunk_count);

        if (response_type == "HSOSSTP_SIDXX") {
            save_file(output_file_name, session_key, response_chunk_count);
        }
    }
}

void Client::request_file(const std::string &file_name, int chunk_size) {
    std::string init_request = "HSOSSTP_INITX;" + std::to_string(chunk_size) + ";" + file_name;
    sendto(sockfd, init_request.c_str(), init_request.size(), 0, reinterpret_cast<sockaddr *>(&server_addr),
           server_len);
    std::cout << "Sent " << init_request.size() << " bytes" << std::endl;
}

void Client::save_file(const std::string &output_file_name, int session_key, int chunk_count) {
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
        std::string data_response(buffer.begin(), buffer.end());
        int chunk_no_received, bytes_read;

        std::cout << "Received data: " << data_response.c_str() << std::endl;
        std::string data_type(13, 0);
        if (sscanf(data_response.c_str(), "%[^;];", data_type.data()) != 1) {
            std::cout << "Error parsing data type" << std::endl;
            break;
        }
        std::cout << "Received data type: " << data_type.c_str() << std::endl;

        if (data_type == "HSOSSTP_DATAX") {
            std::cout << "Data type is correct" << std::endl;
            init_buffer();

            sscanf(data_response.c_str(), "HSOSSTP_DATAX;%d;%d;%s",
                   &chunk_no_received,
                   &bytes_read,
                   buffer.data());

            // rezise buffer to actual size
            buffer.resize(bytes_read);

            std::cout << "Received " << bytes_read << " bytes" << std::endl;
            std::cout << "Received chunk " << chunk_no_received << std::endl;
            std::cout << "Expected chunk " << chunk_no << std::endl;
            std::cout << "Buffer size " << buffer.size() << std::endl;
            std::cout << "Buffer data " << buffer.data() << std::endl;

            if (chunk_no_received != chunk_no) {
                std::cout << "Received wrong chunk" << std::endl;
                break;
            }
            output_file.write(buffer.data(), bytes_read);

            if (chunk_no == chunk_count - 1) {
                break;
            } else {
                chunk_no++;
            }
        } else {
            std::cout << "Received wrong chunk or answere is wrong" << std::endl;
            break;
        }
    }
    std::cout << "File saved" << std::endl;
    output_file.close();
}

// buffer initialization function
void Client::init_buffer() {
    buffer.resize(BUFFER_SIZE);
    std::fill(buffer.begin(), buffer.end(), 0);
}

