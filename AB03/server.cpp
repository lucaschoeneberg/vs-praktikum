//
// Created by Luca Schöneberg on 07.04.23.
//

#include "server.h"

Server::Server() {
    sessions.reserve(100);
}

Server::~Server() {
    close(sockfd);
}

void Server::run() {
    init();
}

/**
 * init initializes the server and starts the main loop
 *
 * @return
 */
[[noreturn]] int Server::init() {
    session_counter = 1;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (const sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    const int buffer_size = 65536;

    while (true) {
        // init buffer
        std::vector<char> buffer(buffer_size);
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int received = recvfrom(sockfd, buffer.data(), buffer_size, 0, (sockaddr *) &client_addr, &client_len);
        if (received > 0) {
            buffer.resize(received);
            handle_request(buffer, client_addr, client_len);
        }
    }
}

/**
 * init_session creates a new session and returns the session key
 *
 * @param chunk_size
 * @param file_name
 * @return
 */
int Server::init_session(int chunk_size, const std::string &file_name) {
    Session session;
    session.chunk_size = chunk_size;
    session.current_chunk = 0;

    session.file_stream.open(file_name, std::ios::binary);
    if (!session.file_stream.is_open()) {
        return -1;
    }

    sessions[session_counter] = std::move(session);

    std::cout << "Created session " << session_counter << std::endl;
    return session_counter++;
}


/**
 * Commands:
 * HSOSSTP_INITX;CHUNK_SIZE;FILE_NAME
 * HSOSSTP_SIDXX;SESSION_KEY;CHUNK_COUNT
 * HSOSSTP_GETXX;SESSION_KEY;CHUNK_NO
 * HSOSSTP_DATAX;CHUNK_NO;BYTES_READ;DATA
 *
 * Responses:
 * HSOSSTP_ERROR;REASON
 * HSOSSTP_SIDXX;SESSION_KEY
 * HSOSSTP_DATAX;SESSION_KEY;CHUNK_NO;BYTES_READ;DATA
 *
 * Reasons:
 * FNF - File not found
 * NOS - No session
 * CNF - Chunk not found
 * ISE - Internal server error
 *
 * handle_request handles the request and sends the response
 *
 * @param request
 * @param client_addr
 * @param client_len
 * @return
 */
void Server::handle_request(const std::vector<char> &request, sockaddr_in &client_addr, socklen_t client_len) {
    std::string request_str(request.begin(), request.end());
    std::string command = request_str.substr(0, 13);
    std::string rest = request_str.substr(14);
    std::istringstream rest_stream(rest);

    if (command == "HSOSSTP_INITX") {
        int chunk_size;
        std::string file_name;
        if (!(rest_stream >> chunk_size >> file_name)) {
            std::string error_msg = "HSOSSTP_ERROR;ISE";
            sendto(sockfd, error_msg.c_str(), error_msg.size(), 0, reinterpret_cast<sockaddr *>(&client_addr),
                   client_len);
            std::cout << "Invalid request: " << request_str << std::endl;
            std::cout << "Sent: " << error_msg << std::endl;
        }
        file_name = file_name.substr(1);

        int session_key = init_session(chunk_size, file_name);
        if (session_key == -1) {
            std::string error_msg = "HSOSSTP_ERROR;FNF";
            sendto(sockfd, error_msg.c_str(), error_msg.size(), 0, reinterpret_cast<sockaddr *>(&client_addr),
                   client_len);
            std::cout << "File not found: " << file_name << std::endl;
            std::cout << "Sent: " << error_msg << std::endl;
        } else {
            std::string response = "HSOSSTP_SIDXX;" + std::to_string(session_key);
            sendto(sockfd, response.c_str(), response.size(), 0, reinterpret_cast<sockaddr *>(&client_addr),
                   client_len);
            std::cout << "Session initialized: " << session_key << std::endl;
            std::cout << "Sent: " << response << std::endl;
        }
    } else if (command == "HSOSSTP_GETXX") {
        int session_key, chunk_no;
        if (sscanf(rest.c_str(), "%d;%d", &session_key, &chunk_no) != 2) {
            std::string error_msg = "HSOSSTP_ERROR;ISE";
            sendto(sockfd, error_msg.c_str(), error_msg.size(), 0, reinterpret_cast<sockaddr *>(&client_addr),
                   client_len);
            std::cout << "Invalid request: " << request_str << std::endl;
            std::cout << "Sent: " << error_msg << std::endl;
            return;
        }

        auto it = sessions.find(session_key);
        if (it == sessions.end()) {
            std::string error_msg = "HSOSSTP_ERROR;NOS";
            sendto(sockfd, error_msg.c_str(), error_msg.size(), 0, reinterpret_cast<sockaddr *>(&client_addr),
                   client_len);
            std::cout << "Sessions: " << sessions.size() << std::endl;
            std::cout << "Session not found: " << session_key << std::endl;
            return;
        }
        auto &session = it->second;
        std::vector<char> buffer(session.chunk_size);
        int bytes_read = read_file(session, buffer.data(), session.chunk_size);
        if (bytes_read < 0) {
            std::string error_msg = "HSOSSTP_ERROR;CNF";
            sendto(sockfd, error_msg.c_str(), error_msg.size(), 0, reinterpret_cast<sockaddr *>(&client_addr),
                   client_len);
            std::cout << "Chunk not found: " << chunk_no << std::endl;
        } else {
            send_response(chunk_no, bytes_read, buffer.data(), client_addr, client_len);
            if (bytes_read < session.chunk_size) {
                session.is_active = false;
                session.file_stream.close();
                sessions.erase(session_key);
            } else {
                session.current_chunk++;
            }
        }
    }
}

/**
 * Sends response to client.
 * Returns number of bytes sent.
 * Returns -1 on error.
 *
 * @param chunk_no
 * @param bytes_read
 * @param buffer
 * @param client_addr
 * @param client_len
 * @return
 */
int Server::send_response(int chunk_no, int bytes_read, const char *buffer, const sockaddr_in &client_addr,
                          socklen_t client_len) const {
    std::ostringstream response;
    response << "HSOSSTP_DATAX;" << chunk_no << ";" << bytes_read << ";";
    response.write(buffer, bytes_read);

    std::string response_str = response.str();
    ssize_t sent = sendto(sockfd, response_str.c_str(), response_str.size(), 0,
                          reinterpret_cast<const sockaddr *>(&client_addr),
                          client_len);
    return static_cast<int>(sent);
}

/**
 * Reads chunk_size bytes from file_stream into buffer.
 * Returns number of bytes read.
 * Returns -1 if file_stream is not open.
 *
 * @param session
 * @param buffer
 * @param chunk_size
 * @return
 */
int Server::read_file(Session &session, char *buffer, const std::streamsize chunk_size) {
    std::ifstream &file_stream = session.file_stream;

    if (!file_stream.is_open()) {
        return -1;
    }

    file_stream.read(buffer, chunk_size);
    return static_cast<int>(file_stream.gcount());
}

/*
int Server::get_file_size(Session &session) const {
    if (!session.file_stream.is_open()) {
        return -1;
    }

    session.file_stream.seekg(0, std::ios::end);
    int size = session.file_stream.tellg();
    session.file_stream.seekg(0, std::ios::beg);
    return size;
}*/
