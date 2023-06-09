//
// Created by Luca Schöneberg on 21.03.23.
//
#include "HTTPServer.h"

HTTPServer::HTTPServer(const std::string &docroot, int port, int max_threads) : docroot_(docroot), port_(port),
                                                                                max_threads_(max_threads),
                                                                                server_socket_(0), running_(false) {}

HTTPServer::~HTTPServer() {
    stop();
}

void HTTPServer::stop() {

}

void HTTPServer::start() {
    struct sockaddr_in serv_addr{};

    // Create socket
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        std::cerr << "ERROR: Failed to create socket." << std::endl;
        return;
    }

    // Bind socket to address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // ToDo: Check if this is correct
    serv_addr.sin_port = htons(port_);
    if (bind(server_socket_, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "ERROR: Failed to bind socket to address." << std::endl;
        close(server_socket_);
        server_socket_ = 0;
        return;
    }

    // Listen for connections
    if (listen(server_socket_, 5) < 0) {
        std::cerr << "ERROR: Failed to listen for connections." << std::endl;
        close(server_socket_);
        server_socket_ = 0;
        return;
    }

    running_ = true;

    // Start worker threads
    // "fork()" ist eine Funktion in Unix-Systemen, die eine neue Prozessinstanz erzeugt, die eine Kopie des
    // ursprünglichen Prozesses ist. Der Hauptunterschied zwischen "fork()" und "std::thread" besteht darin,
    // dass "fork()" eine separate Prozessinstanz erstellt, während "std::thread" einen neuen Thread innerhalb
    // des aktuellen Prozesses erstellt.
    // Die Verwendung von "fork()" kann jedoch zu Problemen führen, insbesondere wenn mehrere Threads in einem
    // Prozess vorhanden sind. Dies liegt daran, dass jeder Thread möglicherweise nicht die gleiche Sicht auf die
    // Ressourcen des Prozesses hat, was zu unvorhersehbarem Verhalten führen kann.
    for (int i = 0; i < max_threads_; ++i) {
        std::thread t(&HTTPServer::worker_thread, this);
        // Das "detach()" auf dem Thread-Objekt bewirkt, dass der Thread unabhängig von seinem
        // Erzeuger-Thread ausgeführt wird.
        t.detach();
    }

    std::cout << "Server started on port " << port_ << "." << std::endl;

    // Wait for stop signal
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Close server socket
    close(server_socket_);
    server_socket_ = 0;
}

void HTTPServer::worker_thread() {
    while (running_) {
        // Accept incoming connection
        struct sockaddr_in cli_addr{};
        socklen_t clilen = sizeof(cli_addr);
        int client_socket = accept(server_socket_, (struct sockaddr *) &cli_addr, &clilen);
        if (client_socket < 0) {
            std::cerr << "ERROR: Failed to accept connection." << std::endl;
            continue;
        }

        // Handle client request
        handle_client(client_socket);

        // Close client socket
        close(client_socket);
    }
}

void HTTPServer::handle_client(int client_socket) {
    // Read client request
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int n = read(client_socket, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        std::cerr << "ERROR: Failed to read from socket." << std::endl;
        return;
    }

    std::string request(buffer);

    // Handle request
    handle_request(client_socket, request);
}

void HTTPServer::handle_request(int client_socket, const std::string &request) {
    std::istringstream iss(request);
    std::string method, path, protocol;
    iss >> method >> path >> protocol;

    // Ignore other headers
    std::string line;
    while (std::getline(iss, line) && !line.empty()) {}

    // Check method
    if (method != "GET" && method != "POST") {
        send_response(client_socket, 501, "Not Implemented", "text/plain", "Method not implemented.");
        return;
    }

    // Normalize path
    if (path.empty() || path == "/") {
        path = "/index.html";
    }
    if (path[0] != '/') {
        path = '/' + path;
    }

    std::string full_path = docroot_ + path;

    // Handle POST request
    if (method == "POST") {
        // Read request body
        std::stringstream ss;
        ss << iss.rdbuf();
        std::string body = ss.str();

        // Parse POST parameters
        std::string param1 = "0", param2 = "0";
        std::istringstream body_iss(line);
        std::string token;
        while (std::getline(body_iss, token, '&')) {
            size_t pos = token.find('=');
            if (pos != std::string::npos) {
                std::string key = token.substr(0, pos);
                std::string value = token.substr(pos + 1);
                if (key == "zahl1") {
                    param1 = value;
                } else if (key == "zahl2") {
                    param2 = value;
                }
            }
        }

        // Compute multiplication result
        int64_t result = std::stoi(param1) * std::stoi(param2);

        // Send response
        std::ostringstream oss;
        oss << "<html><head><title>Ergebnis</title></head><body><center><h1>Ergebnis: " << result
            << "</h1></center></body></html>";
        std::string content = oss.str();
        send_response(client_socket, 200, "OK", "text/html", content);
        return;
    }

    // Check if file or directory exists
    struct stat sb;
    if (stat(full_path.c_str(), &sb) < 0) {
        send_response(client_socket, 404, "Not Found", "text/plain", "File not found.");
        return;
    }

    if (S_ISREG(sb.st_mode)) {
        // File
        std::string content, content_type;
        read_file(full_path, content, content_type);
        send_response(client_socket, 200, "OK", content_type, content);
    } else if (S_ISDIR(sb.st_mode)) {
        // Directory
        std::string content;
        read_dir(full_path, content);
        send_response(client_socket, 200, "OK", "text/html", content);
    } else {
        // Unknown
        send_response(client_socket, 500, "Internal Server Error", "text/plain", "Unknown file type.");
    }
}

// Send HTTP response to client and attach body content
void HTTPServer::send_response(int client_socket, int status_code, const std::string &status_message,
                               const std::string &content_type, const std::string &body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status_code << " " << status_message << "\r\n";
    oss << "Content-Type: " << content_type << "\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n";
    oss << "\r\n";
    oss << body;

    std::string response = oss.str();

    if (write(client_socket, response.c_str(), response.size()) < 0) {
        std::cerr << "ERROR: Failed to write to socket." << std::endl;
    }
}

// Read file content and determine content type
void HTTPServer::read_file(const std::string &path, std::string &content, std::string &content_type) {
    std::ifstream ifs(path, std::ios::binary);

    if (!ifs) {
        throw std::runtime_error("Failed to open file.");
    }

    std::vector<char> buffer(1024);
    std::ostringstream oss;

    while (ifs.read(buffer.data(), buffer.size())) {
        oss.write(buffer.data(), buffer.size());
    }

    if (ifs.gcount() > 0) {
        oss.write(buffer.data(), ifs.gcount());
    }

    content = oss.str();
    content_type = "text/plain";

    std::string extension = get_file_extension(path);
    if (extension == "html" || extension == "htm") {
        content_type = "text/html";
    } else if (extension == "jpg" || extension == "jpeg") {
        content_type = "image/jpeg";
    } else if (extension == "png") {
        content_type = "image/png";
    } else if (extension == "gif") {
        content_type = "image/gif";
    } else if (extension == "css") {
        content_type = "text/css";
    } else if (extension == "js") {
        content_type = "application/javascript";
    } else if (extension == "ico") {
        content_type = "image/x-icon";
    } else if (extension == "txt") {
        content_type = "text/plain";
    } else if (extension == "pdf") {
        content_type = "application/pdf";
    } else if (extension == "json") {
        content_type = "application/json";
    }
}

// Read directory content
void HTTPServer::read_dir(const std::string &path, std::string &content) {
    std::string html_path = path.substr(docroot_.size());
    content = "<html><head><title>" + html_path + "</title></head><body><h1>" + html_path + "</h1><ul>";

    DIR *dir = opendir(path.c_str());
    if (!dir) {
        throw std::runtime_error("Failed to open directory.");
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (std::strcmp(ent->d_name, ".") == 0 || std::strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        std::string name(ent->d_name);
        std::string full_path = path + '/' + name;

        struct stat sb;
        if (stat(full_path.c_str(), &sb) < 0) {
            continue;
        }

        content += "<li><a href=\"" + html_path + '/' + name + "\">" + name + "</a>";
        if (S_ISDIR(sb.st_mode)) {
            content += "/";
        }
        content += "</li>";
    }

    closedir(dir);

    content += "</ul></body></html>";
}

// Get file extension
std::string HTTPServer::get_file_extension(const std::string &path) {
    std::size_t pos = path.rfind('.');
    if (pos == std::string::npos) {
        return "";
    } else {
        return path.substr(pos + 1);
    }
}