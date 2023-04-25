/* Pub-Sub-Server  
 * Getestet unter Ubuntu 20.04 64 Bit / g++ 9.3
 */

#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <set>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

// Diese Includes werden generiert.
#include "pub_sub.grpc.pb.h"
#include "pub_sub_deliv.grpc.pb.h"
#include "pub_sub_config.h"

// Notwendige gRPC Klassen.
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

// Diese Klassen sind in den .proto Dateien definiert.
using pubsub::EmptyMessage;
using pubsub::Message;

using pubsub::PubSubDelivService;
using pubsub::PubSubService;
using pubsub::ReturnCode;
using pubsub::SubscriberAddress;
using pubsub::Topic;

/* Aufgabe 1: Implementierung des Servers
 * Der eigentliche Server (im folgenden auch Dispatcher genannt) sorgt für die Nachrichten-
 * Verteilung und implementiert das Interface pub_sub.proto. Es enthält Funktionen zur
 * Registration / Deregistration von Empfängern, die in einer geeigneten Datenstruktur
 * verwaltet werden, sowie zum Veröffentlichen von Nachrichten. Per set_topic() kann ein
 * Topic gesetzt werden, welches in der ausgelieferten Nachricht jeweils mitgeschickt wird.
 * Nachrichten können per Aufruf von publish() verschickt werden. Das Setzen eines Topics
 * soll nur möglich sein, wenn zuvor ein korrekter Passcode eingegeben wurde.
 */
// Implementierung des Service
class PubSubServiceImpl final : public PubSubService::Service {
    // TODO: Channel topic und Subscribers für diesen Server merken
    std::string topic;
    std::map<std::string, std::unique_ptr<PubSubDelivService::Stub>> subscribers;
    std::string passcode = "1234";


    static std::string stringify(const SubscriberAddress &adr) {
        std::string s = adr.ip_address() + ":";
        s += std::to_string(adr.port());
        return s;
    }

    Status subscribe(ServerContext *context, const SubscriberAddress *request,
                     ReturnCode *reply) override {
        std::string receiver = stringify(*request);

        if (subscribers.count(receiver)) {
            reply->set_value(pubsub::ReturnCode_Values_CLIENT_ALREADY_REGISTERED);
        } else {
            auto channel = grpc::CreateChannel(receiver, grpc::InsecureChannelCredentials());
            subscribers.insert({receiver, PubSubDelivService::NewStub(channel)});
            reply->set_value(::pubsub::ReturnCode_Values::ReturnCode_Values_OK);
            std::cout << "Subscriber added: " << receiver << std::endl;
        }
        return Status::OK;
    }

    Status unsubscribe(ServerContext *context, const SubscriberAddress *request,
                       ReturnCode *reply) override {
        std::string receiver = stringify(*request);

        if (subscribers.count(receiver) == 0) {
            reply->set_value(pubsub::ReturnCode_Values_CANNOT_UNREGISTER);
        } else {
            subscribers.erase(receiver);
            reply->set_value(::pubsub::ReturnCode_Values::ReturnCode_Values_OK);
            std::cout << "Subscriber removed: " << receiver << std::endl;
        }
        return Status::OK;
    }

    void handle_status(const std::string operation, Status &status) {
        // Status auswerten -> deliver() gibt keinen Status zurück,k deshalb nur RPC Fehler melden.
        if (!status.ok()) {
            std::cout << "[ RPC error: " << status.error_code() << " (" << status.error_message()
                      << ") ]" << std::endl;
        }
    }

    Status publish(ServerContext *context, const Message *request,
                   ReturnCode *reply) override {
        for (const auto &subscriber: subscribers) {
            // Nicht mehr notwendig da bereits in subscribe erstellt
            // auto channel = grpc::CreateChannel(subscriber, grpc::InsecureChannelCredentials());
            // std::unique_ptr <PubSubDelivService::Stub> stub = PubSubDelivService::NewStub(channel);

            ClientContext client_context;
            EmptyMessage response;

            Status status = subscriber.second->deliver(&client_context, *request, &response);
            handle_status("publish", status);
        }

        reply->set_value(::pubsub::ReturnCode_Values::ReturnCode_Values_OK);
        return Status::OK;
    }

    Status set_topic(ServerContext *context, const Topic *request,
                     ReturnCode *reply) override {
        if (request->passcode() == passcode) {
            topic = request->topic();
            std::cout << "Topic set to: " << topic << std::endl;
            reply->set_value(::pubsub::ReturnCode_Values::ReturnCode_Values_OK);
        } else {
            std::cout << "Incorrect passcode provided" << std::endl;
            reply->set_value(::pubsub::ReturnCode_Values::ReturnCode_Values_CANNOT_SET_TOPIC);
        }

        return Status::OK;
    }

public:
    PubSubServiceImpl() {
        topic = "<no topic set>";
    }
};

void RunServer() {
    // Server auf dem lokalen Host starten.
    // std::string server_address(PUBSUB_SERVER_IP);
    std::string server_address("0.0.0.0"); // muss der lokale Rechner sein
    server_address += ":";
    server_address += std::to_string(PUBSUB_SERVER_PORT); // Port könnte umkonfiguriert werden

    PubSubServiceImpl service;

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    // Server starten ohne Authentifizierung
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Registrierung als synchroner Dienst
    builder.RegisterService(&service);
    // Server starten
    std::unique_ptr <Server> server(builder.BuildAndStart());
    std::cout << "[ Server launched on " << server_address << " ]" << std::endl;

    // Warten auf das Ende Servers. Das muss durch einen anderen Thread
    // ausgelöst werden.  Alternativ kann der ganze Prozess beendet werden.
    server->Wait();
}

int main(int argc, char **argv) {
    // Server starten
    RunServer();
    return 0;
}