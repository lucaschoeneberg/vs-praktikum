//
// Created by Luca Schöneberg on 21.03.23.
//

#include "HTTPServer.h"

http_server::http_server() {
    this->port = 0;
    this->docroot = "";
    this->max_threads = 0;
}

http_server::~http_server() {
}

void http_server::init(int port, std::string docroot, int max_threads) {
    this->port = port;
    this->docroot = docroot;
    this->max_threads = max_threads;



}
