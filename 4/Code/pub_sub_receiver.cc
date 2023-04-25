/* Pub-Sub-Server  
 * Getestet unter Ubuntu 20.04 64 Bit / g++ 9.3
 * @hje
 */

#include <iostream>
#include <memory>
#include <string>
#include <fstream>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

// Dieses Include wird generiert.
#include "pub_sub_deliv.grpc.pb.h"

// Notwendige gRPC Klassen.
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

// Diese Klassen sind in den .proto Dateien definiert.
using pubsub::Message;
using pubsub::EmptyMessage;
using pubsub::PubSubDelivService;

// Implementierung des Service
class PubSubDelivServiceImpl final : public PubSubDelivService::Service {
    Status deliver(ServerContext *context, const Message *request,
                   EmptyMessage *reply) override {

        // Get the current time
        auto now = std::chrono::system_clock::now();
        // Convert to a time_t object
        std::time_t now_c = std::chrono::system_clock::to_time_t(now)
        // Convert to a string
        std::string time_string = std::ctime(&now_c);
        // Remove the trailing newline character
        time_string.pop_back();

        // Check if the string is empty
        if (request->message().empty()) {
            // Print the message and timestamp to the console
            std::cout << "Received empty message at " << time_string << std::endl;
        } else{
            // Print the message and timestamp to the console
            std::cout << "Received message \"" << request->message() << "\" at " << time_string << std::endl;
        }

        return Status::OK;
    }
};

void RunServer() {
    // Server auf dem lokalen Host starten.
    std::string server_address("0.0.0.0:40041");
    PubSubDelivServiceImpl service;

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    // Server starten ohne Authentifizierung
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Registrierung als synchroner Dienst
    builder.RegisterService(&service);
    // Server starten
    std::unique_ptr <Server> server(builder.BuildAndStart());
    std::cout << "Server gestartet auf " << server_address << std::endl;

    // Warten auf das Ende Servers. Das muss durch einen anderen Thread
    // ausgeloest werden.  Alternativ kann der ganze Prozess beendet werden.
    server->Wait();
}

int main(int argc, char **argv) {
    // Server starten
    RunServer();
    return 0;
}