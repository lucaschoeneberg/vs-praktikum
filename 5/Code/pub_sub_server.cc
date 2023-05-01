/* Pub-Sub-Server  
 * Getestet unter Ubuntu 20.04 64 Bit / g++ 9.3
 */

#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <set>
#include <openssl/evp.h>
#include <random>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <iomanip>

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
using pubsub::PubSubParam;

#define USERLEN 12
#define PWDLEN 12
#define HASHLEN 64
#define MAX_HASH_DIGEST_LENGTH 100

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
    std::string topic;
    std::map<std::string, std::unique_ptr<PubSubDelivService::Stub>> subscribers;
    std::map<int32_t, std::string> active_sessions;
    std::map<std::string, std::string> credentials;
    std::string passcode = "1234";

    static std::string hash(const std::string &s) {
        std::hash<std::string> hasher;
        return std::to_string(hasher(s));
    }

    static std::string stringify(const SubscriberAddress &adr) {
        std::string s = adr.ip_address() + ":";
        s += std::to_string(adr.port());
        return s;
    }

    Status subscribe(ServerContext *context, const PubSubParam *request,
                     ReturnCode *reply) override {
        std::cout << "Attempting to subscribe" << std::endl;
        std::string receiver = stringify(request->optaddress());

        if (subscribers.count(receiver)) {
            reply->set_value(pubsub::ReturnCode_Values_CLIENT_ALREADY_REGISTERED);
            std::cout << "Subscriber already registered: " << receiver << std::endl;
        } else if (!check_session(request->sid().id(),
                                  reinterpret_cast<std::string *>(*request->optaddress().ip_address().c_str()),
                                  &request->hash_string(),
                                  reply)) {
            auto channel = grpc::CreateChannel(receiver, grpc::InsecureChannelCredentials());
            subscribers.insert({receiver, PubSubDelivService::NewStub(channel)});
            reply->set_value(::pubsub::ReturnCode_Values::ReturnCode_Values_OK);
            std::cout << "Subscriber added: " << receiver << std::endl;
        }

        std::cout << std::endl;
        return Status::OK;
    }

    Status unsubscribe(ServerContext *context, const PubSubParam *request,
                       ReturnCode *reply) override {
        std::cout << "Attempting to unsubscribe" << std::endl;
        std::string receiver = stringify(request->optaddress());

        if (subscribers.count(receiver) == 0) {
            reply->set_value(pubsub::ReturnCode_Values_CANNOT_UNREGISTER);
            std::cout << "Subscriber not found: " << receiver << std::endl;
        } else {
            subscribers.erase(receiver);
            reply->set_value(::pubsub::ReturnCode_Values::ReturnCode_Values_OK);
            std::cout << "Subscriber removed: " << receiver << std::endl;
        }
        std::cout << std::endl;
        return Status::OK;
    }

    void handle_status(const std::string &operation, Status &status) {
        // Status auswerten -> deliver() gibt keinen Status zurück,k deshalb nur RPC Fehler melden.
        if (!status.ok()) {
            std::cout << "[ RPC error: " << status.error_code() << " (" << status.error_message()
                      << ") ]" << std::endl;
        }
    }

    Status publish(ServerContext *context, const PubSubParam *request,
                   ReturnCode *reply) override {
        std::cout << "Publishing message: " << request->optmessage().message() << std::endl;
        std::cout << "Topic: " << topic << std::endl;
        std::cout << "Subscribers: " << subscribers.size() << std::endl;
        for (const auto &subscriber: subscribers) {
            // Nicht mehr notwendig da bereits in subscribe erstellt
            // auto channel = grpc::CreateChannel(subscriber, grpc::InsecureChannelCredentials());
            // std::unique_ptr <PubSubDelivService::Stub> stub = PubSubDelivService::NewStub(channel);

            ClientContext client_context;
            EmptyMessage response;

            std::cout << "Delivering message to: " << subscriber.first << std::endl;
            Status status = subscriber.second->deliver(&client_context, request->optmessage(), &response);
            handle_status("publish", status);
        }

        reply->set_value(::pubsub::ReturnCode_Values::ReturnCode_Values_OK);

        std::cout << std::endl;
        return Status::OK;
    }

    Status set_topic(ServerContext *context, const PubSubParam *request,
                     ReturnCode *reply) override {
        std::cout << "Setting topic to: " << request->opttopic().passcode() << std::endl;
        std::string _passcode = request->opttopic().passcode();
        if (!check_session(request->sid().id(), &_passcode, &request->hash_string(), reply)) {
            std::cout << "Session not found" << std::endl;
            return Status::OK;
        }
        std::cout << std::endl;
        if (_passcode == passcode) {
            topic = request->opttopic().topic();
            std::cout << "Topic set to: " << topic << std::endl;
            reply->set_value(::pubsub::ReturnCode_Values::ReturnCode_Values_OK);
        } else {
            std::cout << "Incorrect passcode provided" << std::endl;
            reply->set_value(::pubsub::ReturnCode_Values::ReturnCode_Values_CANNOT_SET_TOPIC);
        }
        std::cout << std::endl;
        return Status::OK;
    }

    Status get_session(ServerContext *context, const pubsub::UserName *request, pubsub::SessionId *response) override {
        // Hier sollte eine sicherere Methode zur Generierung von Sitzungs-IDs implementiert werden
        int32_t session_id = generate_session_id();
        active_sessions[session_id] = request->name();
        response->set_id(session_id);
        std::cout << "Session created for user: " << request->name() << ", session ID: " << session_id << std::endl;
        return Status::OK;
    }

    Status validate(ServerContext *context, const pubsub::PubSubParam *request, ReturnCode *reply) override {
        int32_t session_id = request->sid().id();
        std::string hash_string = request->hash_string();

        if (!check_session(session_id, nullptr, &hash_string, reply)) {
            return Status::OK;
        }

        std::cout << "Session validated: " << session_id << std::endl;
        reply->set_value(pubsub::ReturnCode_Values_OK);
        return Status::OK;
    }

    Status invalidate(ServerContext *context, const pubsub::SessionId *request, ReturnCode *reply) override {
        int32_t session_id = request->id();
        if (active_sessions.erase(session_id)) {
            reply->set_value(pubsub::ReturnCode_Values_OK);
            std::cout << "Session invalidated: " << session_id << std::endl;
        } else {
            reply->set_value(pubsub::ReturnCode_Values_SESSION_INVALID);
            std::cout << "Invalid session ID: " << session_id << std::endl;
        }
        return Status::OK;
    }

private:
    bool check_session(int session_id, std::string *data, const std::string *msgHashValue, ReturnCode *reply) {
        if (active_sessions.find(session_id) == active_sessions.end()) {
            std::cout << "Session ID not found: " << session_id << std::endl;
            reply->set_value(pubsub::ReturnCode_Values_SESSION_INVALID);
            return false;
        }

        std::string s_name = active_sessions.at(session_id);

        if (credentials.find(s_name) == credentials.end()) {
            std::cerr << "<info> client hat versucht sich mit nicht existierenden Benutzer : " << s_name
                      << " anzumelden" << std::endl;
            reply->set_value(ReturnCode::SESSION_INVALID);
            return false;
        }

        std::string pw = credentials.at(s_name);
        std::cout << "Credentials: " << s_name << " : " << pw << std::endl;

        // check if hash length is exact
        if (msgHashValue->length() == 64)
            // check if hash is equal to hash of message
            if (hash_sha(std::to_string(session_id) + hash_sha((data == nullptr) ? pw : *data + pw)) == *msgHashValue) {
                std::cout << "Session validated: " << session_id << std::endl;
                return true;
            } else {
                std::cout << "Session invalid: " << session_id << std::endl;
                reply->set_value(pubsub::ReturnCode_Values_NO_VALID_HASH);
                return false;
            }
        else {
            std::cout << "Session invalid: " << session_id << std::endl;
            reply->set_value(pubsub::ReturnCode_Values_SESSION_INVALID);
            return false;
        }
    }

    static std::string hash_sha(const std::string input) {
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hash_length;

        const EVP_MD *md = EVP_sha256();
        EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

        EVP_DigestInit_ex(mdctx, md, nullptr);
        EVP_DigestUpdate(mdctx, input.c_str(), input.size());
        EVP_DigestFinal_ex(mdctx, hash, &hash_length);
        EVP_MD_CTX_free(mdctx);

        std::stringstream ss;
        for (unsigned int i = 0; i < hash_length; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }

        return ss.str();
    }

    void init_hash_digest() {
        // Open the file.
        std::ifstream file("hashes.txt");
        // If the file is not open, print an error message and return.
        if (!file.is_open()) {
            std::cerr << "Cannot open digest 'hashes.txt'.\n";
            return;
        }
        // Read the file line by line.
        std::string line;
        while (std::getline(file, line)) {
            char user[USERLEN + 1];
            char hash[HASHLEN + 1];
            // Parse the line and store the user and hash in separate character arrays.
            int n = sscanf(line.c_str(), "%s %s", user, hash);
            if (n == 2) {
                // Convert the character arrays to strings.
                std::string userStr(user);

                // check if hash has length
                if (strlen(hash) != HASHLEN) {
                    std::cerr << "Hash length not correct.\n";
                    continue;
                }
                std::string hashStr(hash);
                // Insert the user and hash into the global credentials map.
                credentials.insert({userStr, hashStr});
            }
        }

        // Close the file.
        file.close();
    }

    static int32_t generate_session_id() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int32_t> dist(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());

        int32_t session_id = dist(gen);
        return session_id;
    }

public:
    PubSubServiceImpl() {
        topic = "<no topic set>";
        init_hash_digest();
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
    std::unique_ptr<Server> server(builder.BuildAndStart());
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