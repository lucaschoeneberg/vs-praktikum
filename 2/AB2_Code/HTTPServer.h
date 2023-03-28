//
// Created by Luca Schöneberg on 21.03.23.
//

#ifndef AB2_CODE_HTTPSERVER_H
#define AB2_CODE_HTTPSERVER_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <dirent.h>

#include <string>

/*
1. Ihr Programm soll als mehrstufiger Server ausgelegt sein.
2. Der Server wird unter Angabe eines Ports und eines Verzeichnisses, in dem html-
Dokumente gespeichert sind, gestartet:
HTTPserv <docroot> <port>
3. Bekanntlich wird üblicherweise der Port 80 verwendet, dessen Benutzung aber spezielle
Privilegien erfordert. Verwenden Sie deshalb Port 8080 oder 8008.
4. Der Server muss nur die Verarbeitung von GET-Requests unterstützen:
a. Die in dem mit dem Request übermittelten URI angegebene Datei wird in dem Arbeitsverzeichnis gesucht, ausgelesen und an den aufrufenden Client zurückgegeben (Statuscode 200). Kann die in der URI angegebene Datei nicht gefunden werden, wird der Statuscode 404 zurückgegeben.
b. Wird in der URI ein Verzeichnis (Unterverzeichnis im Arbeitsverzeichnis des Servers) referenziert, so wird das Verzeichnis in Form eines html-Dokumentes ausgegeben. Die in dem Verzeichnis vorhandenen html-Dateien und Unterverzeichnisse sind in dem Dokument verlinkt.
5. Sehen Sie die korrekte Behandlung von Dateitypen (z.B. Bilddateien im jpg-Format) vor. Dazu muss der Content-Type im Response-Header angepasst werden und Sie müssen die Dateien binär übertragen.
6. Im Dateibereich der Veranstaltung finden Sie ein Beispiel-Verzeichnis, welches durch Ihren Server abrufbar sein soll.
7. Sehen Sie Testausgaben zwecks Kontrolle der Verarbeitung der Client-Anfragen vor und achten Sie auf eine angemessene Fehlerbehandlung.
8. Testen Sie Ihren Server mit mehr als einem Browser und notieren Sie die Unterschiede bei der Verarbeitung von Anfragen.
*/

// HTTPServer class definition
class HTTPServer {
public:
    // Constructor that takes the server's document root directory, port number, and maximum number of threads
    HTTPServer(const std::string &docroot, int port = 8080, int max_threads = 10);

    // Destructor to stop the server
    ~HTTPServer();

    // Method to start the server
    void start();

    // Method to stop the server
    void stop();

private:
    // Private method that handles an incoming client request
    void handle_client(int client_socket);

    // Private method that handles an HTTP request and sends the corresponding response
    void handle_request(int client_socket, const std::string &request);

    // Private method that sends an HTTP response to the client
    void send_response(int client_socket, int status_code, const std::string &status_message,
                       const std::string &content_type, const std::string &body);

    // Private method that reads the contents of a file and determines its content type
    void read_file(const std::string &path, std::string &content, std::string &content_type);

    // Private method that reads the contents of a directory and constructs an HTML page with links to its contents
    void read_dir(const std::string &path, std::string &content);

    // Private method that returns the file extension of a file path
    std::string get_file_extension(const std::string &path);

    // Private method that worker threads use to handle incoming client requests
    void worker_thread();

    // Member variables to store the server's document root directory, port number, and maximum number of threads
    std::string docroot_;
    int port_;
    int max_threads_;

    // Member variables to store the server socket and running state
    int server_socket_;
    bool running_;

};


#endif //AB2_CODE_HTTPSERVER_H
