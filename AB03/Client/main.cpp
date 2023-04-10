#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "client.h"

int main() {
    Client client("127.0.0.1", 8999);

    client.fetch_file(".ninja_log", 100, "test.txt");

    return 0;

    int sockfd;
    sockaddr_in server_addr;
    const int PORT = 8999;

    std::string server_ip = "127.0.0.1";
    int chunk_size = 100;
    std::string file_name = "test.txt";

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

    std::string message = "HSOSSTP_INITX;" + std::to_string(chunk_size) + ";" + file_name;
    sendto(sockfd, message.c_str(), message.length(), 0, (const sockaddr*)&server_addr, sizeof(server_addr));

    char buffer[2048];
    socklen_t len = sizeof(server_addr);
    int n = recvfrom(sockfd, buffer, 2048, 0, (sockaddr*)&server_addr, &len);
    buffer[n] = '\0';

    int session_key;
    sscanf(buffer, "HSOSSTP_SIDXX;%d", &session_key);

    // Öffne lokale Datei zum Schreiben
    std::ofstream local_file(file_name, std::ios::binary);

    int chunk_no = 0;
    while (true) {
        message = "HSOSSTP_GETXX;" + std::to_string(session_key) + ";" + std::to_string(chunk_no);
        sendto(sockfd, message.c_str(), message.length(), 0, (const sockaddr*)&server_addr, sizeof(server_addr));

        n = recvfrom(sockfd, buffer, 2048, 0, (sockaddr*)&server_addr, &len);
        buffer[n] = '\0';

        int actual_chunk_size;
        char data[2048];
        sscanf(buffer, "HSOSSTP_DATAX;%d;%d;%s", &chunk_no, &actual_chunk_size, data);

        if (actual_chunk_size == 0) {
            break;
        }

        local_file.write(data, actual_chunk_size);
        chunk_no++;
    }

    local_file.close();
    close(sockfd);
    return 0;
}