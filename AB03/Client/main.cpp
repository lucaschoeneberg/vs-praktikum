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

    client.fetch_file("test.txt", 100, "test.txt");

    return 0;
}