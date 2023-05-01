/* Pub-Sub-Client
 * Implementiert eine interaktive Shell, in die Kommandos eingegeben werden können.
 * Getestet unter Ubuntu 20.04 64 Bit / g++ 9.3
 * @hje
 */

#include <iostream>
#include <memory>
#include <string>
#include <cstdio>
#include <openssl/evp.h>
#include <grpcpp/grpcpp.h>

// Diese Includes werden generiert.
#include "pub_sub.grpc.pb.h"
#include "pub_sub_common.grpc.pb.h"
#include "pub_sub_config.h"

#include <unistd.h>
#include <iomanip>

// Notwendige gRPC Klassen im Client.
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

// Diese Klassen sind in den .proto Dateien definiert.
using pubsub::EmptyMessage;
using pubsub::Message;
using pubsub::SubscriberAddress;
using pubsub::PubSubService;
using pubsub::ReturnCode;
using pubsub::Topic;
using pubsub::PubSubParam;

/**** Dies muss editiert werden! ****/
char receiverExecFile[] = RECEIVER_EXEC_FILE;

void trim(std::string &s) {
    /* erstes '\n' durch '\0' ersetzen */
    for (char &i: s) {
        if (i == '\n') {
            i = '\0';
            break;
        }
    }
}

// Argumente für Aufruf: der Client kann mit --target aufgerufen werden.
class Args {
public:
    std::string target;

    Args(int argc, char **argv) {
        target = PUBSUB_SERVER_IP;
        target += ":";
        target += std::to_string(PUBSUB_SERVER_PORT);

        // Endpunkt des Aufrufs ueber --target eingestellt?
        std::string arg_str("--target");
        for (int i = 1; i < argc; i++) {
            std::string arg_val = argv[i];
            size_t start_pos = arg_val.find(arg_str);
            if (start_pos != std::string::npos) {
                start_pos += arg_str.size();
                if (arg_val[start_pos] == '=') {
                    target = arg_val.substr(start_pos + 1);
                } else {
                    std::cout << "Error: set server address via --target=" << std::endl;
                    std::cout << target << " will be used instead." << std::endl;
                }
            }
        }
    }
};

static std::string get_receiver_ip() {
    // Hier wird eine statisch konfigurierte Adresse zurueck gegeben. 
    // Diese koennte auch dynamisch ermittelt werden. 
    // Dann aber: alle Adapter und die dafuer vorgesehenen IP Adresse durchgehen; 
    // eine davon auswaehlen. Das funktioniert aber auch nur im lokalen Netz.
    // Was ist, wenn NAT verwendet wird? Oder Proxies?
    return PUBSUB_RECEIVER_IP;
}

class PubSubClient {

private:
    static void print_prompt(const Args &args) {
        std::cout << "Pub / sub server is: " << args.target << std::endl;
    }

    static void print_help() {
        std::cout << "Client usage: \n";
        std::cout << "     'login' to login;\n";
        std::cout << "     'logout' to logout (if logged in);\n";
        std::cout << "     'quit' to exit;\n";
        std::cout << "     'set_topic' to set new topic;\n";
        std::cout << "     'subscribe' subscribe to server & register / start receiver;\n";
        std::cout << "     'unsubscribe' from this server & terminate receiver.\n";
    }

    /*  OK = 0;
        CANNOT_REGISTER = 1;
        CLIENT_ALREADY_REGISTERED = 2;
        CANNOT_UNREGISTER = 3;
        CANNOT_SET_TOPIC = 4;
        NO_HASH_FOR_SESSION = 5;
        NO_VALID_HASH = 6;
        WRONG_HASH_FOR_SESSION = 7;
        USER_ALREADY_LOGGED_IN = 8;
        SESSION_INVALID = 9;
        UNKNOWN_ERROR = 10;
     */
    static std::string stringify(pubsub::ReturnCode_Values value) {
        switch (value) {
            case pubsub::ReturnCode_Values_OK:
                return "OK";
            case pubsub::ReturnCode_Values_CANNOT_REGISTER:
                return "CANNOT_REGISTER";
            case pubsub::ReturnCode_Values_CLIENT_ALREADY_REGISTERED:
                return "CLIENT_ALREADY_REGISTERED";
            case pubsub::ReturnCode_Values_CANNOT_UNREGISTER:
                return "CANNOT_UNREGISTER";
            case pubsub::ReturnCode_Values_CANNOT_SET_TOPIC:
                return "CANNOT_SET_TOPIC";
            case pubsub::ReturnCode_Values_NO_HASH_FOR_SESSION:
                return "NO_HASH_FOR_SESSION";
            case pubsub::ReturnCode_Values_NO_VALID_HASH:
                return "NO_VALID_HASH";
            case pubsub::ReturnCode_Values_WRONG_HASH_FOR_SESSION:
                return "WRONG_HASH_FOR_SESSION";
            case pubsub::ReturnCode_Values_USER_ALREADY_LOGGED_IN:
                return "USER_ALREADY_LOGGED_IN";
            case pubsub::ReturnCode_Values_SESSION_INVALID:
                return "SESSION_INVALID";
            default:
                return "UNKNOWN_ERROR";
        }
    }

    static std::string hash_sha(const std::string &input) {
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hash_length;

        const EVP_MD *md = EVP_sha256();
        EVP_MD_CTX *mdCtx = EVP_MD_CTX_new();

        EVP_DigestInit_ex(mdCtx, md, nullptr);
        EVP_DigestUpdate(mdCtx, input.c_str(), input.size());
        EVP_DigestFinal_ex(mdCtx, hash, &hash_length);
        EVP_MD_CTX_free(mdCtx);

        std::stringstream ss;
        for (unsigned int i = 0; i < hash_length; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }

        return ss.str();
    }

    static std::string generate_hash(const int session_id, const std::string &data, const std::string &password) {
        // HASH: hash_sha(session_id + hash_sha(username + password))
        std::string hash = std::to_string(session_id) + hash_sha(data + password);
        return hash_sha(hash);
    }

    static std::string generate_loginHash(const pubsub::UserName &username, const std::string &password) {
        // HASH: hash_sha(username + password)
        return hash_sha(username.name() + password);
    }

    static void handle_status(const std::string &operation, Status &status, ReturnCode &reply) {
        // Status auswerten
        if (status.ok()) {
            std::cout << operation << " -> " << stringify(reply.value()) << std::endl;
        } else {
            std::cout << "RPC error: " << status.error_code() << " (" << status.error_message()
                      << ")" << std::endl;
        }
    }

    static void handle_status(const std::string &operation, Status &status) {
        if (!status.ok()) {
            std::cout << "RPC error: " << status.error_code() << " (" << status.error_message()
                      << ")" << std::endl;
        } else {
            std::cout << operation << " -> OK" << std::endl;
        }
    }

public:
    explicit PubSubClient(const std::shared_ptr<Channel> &channel)
            : stub_(PubSubService::NewStub(channel)) {
    }

    void run_shell(const Args &args) {
        /* PID of the Receiver Console */
        int rec_pid = -1;

        print_prompt(args);
        print_help();

        std::string cmd;
        do {
            std::cout << "> ";
            // Read input line
            getline(std::cin, cmd);
            trim(cmd);
            if (cmd.length() == 0)
                continue;

            if (cmd == "login") {
                // Initialize Client Context
                ClientContext session_context;
                // Placeholder for Request & Reply.

                // Read username & password
                pubsub::UserName user;
                std::cout << "enter user> ";
                std::string user_;
                getline(std::cin, user_);
                trim(user_);
                user.set_name(user_);

                // Get Session ID
                auto *session_id = new pubsub::SessionId();
                Status status = stub_->get_session(&session_context, user, session_id);
                PubSubClient::sessionID = session_id->id();
                handle_status("getSessionID()", status);
                if (!status.ok()) {
                    continue;
                }

                // Read password
                std::cout << "enter password> ";
                std::string password;
                getline(std::cin, password);
                trim(password);

                // Calculate hash value
                // Hash(session_id;topic;Hash(user;password))
                loginHash = generate_loginHash(user, password);
                std::cout << "Generated loginHash: " << loginHash << std::endl;

                PubSubParam request;
                request.set_allocated_sid(session_id);
                request.set_allocated_hash_string(new std::string(generate_hash(session_id->id(), "", loginHash)));

                // Make RPC
                ReturnCode reply;
                ClientContext validate_context;
                status = stub_->validate(&validate_context, request, &reply);
                PubSubClient::handle_status("validate()", status, reply);
            } else if (cmd == "logout") {
                ClientContext context;
                ReturnCode reply;
                pubsub::SessionId session;
                session.set_id(sessionID);
                Status status = stub_->invalidate(&context, session, &reply);
                PubSubClient::handle_status("logout", status, reply);
                if (status.ok()) {
                    std::cout << "logout successfull> " << std::endl;
                }
            } else if (cmd == "set_topic") {
                std::string topic;
                std::cout << "enter topic> ";
                getline(std::cin, topic);
                trim(topic);
                // Read passcode to set the topic.
                std::string passcode;
                std::cout << "enter passcode> ";
                getline(std::cin, passcode);
                trim(passcode);

                // Create request
                PubSubParam request;

                // Set topic
                auto *optTopic = new Topic();
                optTopic->set_topic(topic);
                optTopic->set_passcode(passcode);
                request.set_allocated_opttopic(optTopic);

                // Set session ID
                auto *sid = new pubsub::SessionId();
                sid->set_id(sessionID);
                request.set_allocated_sid(sid);

                std::cout << "Data: " << topic + passcode << std::endl;
                // Set hash
                request.set_allocated_hash_string(
                        new std::string(generate_hash(sid->id(), topic + passcode, loginHash)));

                // Context can influence the processing of RPCs. Not used here.
                ClientContext context;
                ReturnCode reply;
                Status status = stub_->set_topic(&context, request, &reply);

                // Handle status / reply
                PubSubClient::handle_status("set_topic()", status, reply);
            } else if (cmd == "subscribe") {
                /* Check if Receiver binary exists */
                if (access(receiverExecFile, X_OK) != -1) {
                    /* Start receiver */
                    if ((rec_pid = fork()) < 0) {
                        std::cerr << "Cannot create process for receiver!\n";
                    } else if (rec_pid == 0) {
                        execl("/usr/bin/xterm", "Receiver", "-fs", "14", receiverExecFile, (char *) nullptr);
                        exit(0); /* End child process */
                    }

                    // Create request
                    PubSubParam request;

                    // Set Subscriber Address
                    auto *optAddress = new SubscriberAddress();
                    optAddress->set_ip_address(PUBSUB_RECEIVER_IP);
                    optAddress->set_port(PUBSUB_RECEIVER_PORT);
                    request.set_allocated_optaddress(optAddress);

                    // Set hash
                    request.set_allocated_hash_string(
                            new std::string(
                                    generate_hash(sessionID, get_receiver_ip() + std::to_string(PUBSUB_RECEIVER_PORT),
                                                  loginHash)));

                    // Set session ID
                    auto *sid = new pubsub::SessionId();
                    sid->set_id(sessionID);
                    request.set_allocated_sid(sid);

                    ClientContext context;
                    ReturnCode reply;

                    Status status = stub_->subscribe(&context, request, &reply);

                    PubSubClient::handle_status("subscribe()", status, reply);
                } else {
                    std::cerr << "Cannot find message receiver executable ("
                              << receiverExecFile << ")!\n";
                    std::cerr << "Press <return> to continue";
                    char c = getc(stdin);
                    continue;
                }
            } else if ((cmd == "quit") ||
                       (cmd == "unsubscribe")) {
                /* Terminate receiver console */
                if (rec_pid > 0) {
                    if (kill(rec_pid, SIGTERM) != 0)
                        std::cerr << "Cannot terminate message receiver!\n";
                    else
                        rec_pid = -1;
                }
                /* An unsubscribe() also needs to be done in the case of "quit". */

                PubSubParam request;
                auto *optaddress = new SubscriberAddress();

                optaddress->set_ip_address(PUBSUB_RECEIVER_IP);
                optaddress->set_port(PUBSUB_RECEIVER_PORT);
                request.set_allocated_optaddress(optaddress);

                // Set hash
                request.set_allocated_hash_string(
                        new std::string(
                                generate_hash(sessionID, get_receiver_ip() + std::to_string(PUBSUB_RECEIVER_PORT),
                                              loginHash)));

                // Set session ID
                auto *sid = new pubsub::SessionId();
                sid->set_id(sessionID);
                request.set_allocated_sid(sid);

                ClientContext context;
                ReturnCode reply;

                Status status = stub_->unsubscribe(&context, request, &reply);
                PubSubClient::handle_status("unsubscribe()", status, reply);

                /* Terminate shell only if "quit" */
                if (cmd == "quit")
                    break; /* Terminate shell */
            } else  /* no command -> call publish() */
            {
                std::string message;
                std::cout << "enter message: > ";
                getline(std::cin, message);
                trim(message);

                // Create request
                PubSubParam request;

                auto *optMessage = new Message();
                optMessage->set_message(message);
                request.set_allocated_optmessage(optMessage);

                // Set hash
                request.set_allocated_hash_string(
                        new std::string(
                                generate_hash(sessionID, message,
                                              loginHash)));

                // Set session ID
                auto *sid = new pubsub::SessionId();
                sid->set_id(sessionID);
                request.set_allocated_sid(sid);

                // Context can influence the processing of RPCs. Not used here.
                ClientContext context;
                ReturnCode reply;
                Status status = stub_->publish(&context, request, &reply);

                PubSubClient::handle_status("publish()", status, reply);
            }
        } while (true);
    }

private:
    std::unique_ptr<PubSubService::Stub> stub_;
    int32_t sessionID;
    std::string loginHash;
};

int main(int argc, char **argv) {
    // Version of GRPC and Protobuf
    std::cout << "GRPC version: " << grpc::Version() << std::endl;
    std::cout << "Protobuf version: " << GOOGLE_PROTOBUF_VERSION << std::endl;
    // Einlesen der Argumente. Der Endpunkt des Aufrufs
    // kann über die Option --target eingestellt werden.
    Args args(argc, argv);
    PubSubClient client(grpc::CreateChannel(args.target, grpc::InsecureChannelCredentials()));

    client.run_shell(args);

    return 0;
}