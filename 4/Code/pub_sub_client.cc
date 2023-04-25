/* Pub-Sub-Client
 * Implementiert eine interaktive Shell, in die Kommandos eingegeben werden können.
 * Getestet unter Ubuntu 20.04 64 Bit / g++ 9.3
 * @hje
 */

#include <iostream>
#include <memory>
#include <string>

#include <stdio.h>
#include <grpcpp/grpcpp.h>

// Diese Includes werden generiert.
#include "pub_sub.grpc.pb.h"
#include "pub_sub_common.grpc.pb.h"
#include "pub_sub_config.h"

#include <unistd.h>

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

/**** Dies muss editiert werden! ****/
std::string receiverExecFile = RECEIVER_EXEC_FILE;

/* TODO: noch notwendig? */
// Die trim-Funktion ist nicht unbedingt erforderlich,
// weil sie lediglich das erste Vorkommen von '\n' durch '\0' ersetzt. Da wir nun
// std::getline zum Lesen der Eingabezeilen verwenden, wird das abschließende
// '\n'-Zeichen automatisch entfernt. Daher ist die trim-Funktion in diesem Fall
// nicht notwendig.
void trim(std::string &s) {
    /* erstes '\n' durch '\0' ersetzen */
    for (int i = 0; i < s.length(); i++) {
        if (s[i] == '\n') {
            s[i] = '\0';
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
    // Was ist wenn NAT verwendet wird? Oder Proxies? 
    return PUBSUB_RECEIVER_IP;
}

class PubSubClient {
private:
    void print_prompt(const Args &args) {
        std::cout << "Pub / sub server is: " << args.target << std::endl;
    }

    void print_help() {
        std::cout << "Client usage: \n";
        std::cout << "     'quit' to exit;\n";
        std::cout << "     'set_topic' to set new topic;\n";
        std::cout << "     'subscribe' subscribe to server & register / start receiver;\n";
        std::cout << "     'unsubscribe' from this server & terminate receiver.\n";
    }

    std::string stringify(pubsub::ReturnCode_Values value) {
        switch (value) {
            case pubsub::ReturnCode_Values_OK:
                return "OK";
            case pubsub::ReturnCode_Values_ERROR:
                return "ERROR";
            case pubsub::ReturnCode_Values_TOPIC_NOT_SET:
                return "TOPIC_NOT_SET";
            case pubsub::ReturnCode_Values_TOPIC_ALREADY_SET:
                return "TOPIC_ALREADY_SET";
            case pubsub::ReturnCode_Values_TOPIC_NOT_FOUND:
                return "TOPIC_NOT_FOUND";
            case pubsub::ReturnCode_Values_SUBSCRIBER_ALREADY_REGISTERED:
                return "SUBSCRIBER_ALREADY_REGISTERED";
            case pubsub::ReturnCode_Values_SUBSCRIBER_NOT_REGISTERED:
                return "SUBSCRIBER_NOT_REGISTERED";
            default:
                return "UNKNOWN";
        }
    }


    void handle_status(const std::string operation, Status &status, ReturnCode &reply) {
        // Status auswerten
        if (status.ok()) {
            std::cout << operation << " -> " << stringify(reply.value()) << std::endl;
        } else {
            std::cout << "RPC error: " << status.error_code() << " (" << status.error_message()
                      << ")" << std::endl;
        }
    }

public:
    PubSubClient(std::shared_ptr <Channel> channel)
            : stub_(PubSubService::NewStub(channel)) {
    }

    void run_shell(const Args &args) {
        /* PID der Receiver Console */
        int rec_pid = -1;

        print_prompt(args);
        print_help();

        std::string cmd;
        do {
            std::cout << "> ";
            // Eingabezeile lesen
            getline(std::cin, cmd);
            // std::cin >> cmd;
            trim(cmd);
            if (cmd.length() == 0)
                continue;

            if (cmd.compare("set_topic") == 0) {
                std::string topic;
                std::cout << "enter topic> ";
                getline(std::cin, topic);
                trim(topic);
                // Passcode einlesen, damit topic gesetzt werden darf.
                std::string passcode;
                std::cout << "enter passcode> ";
                getline(std::cin, passcode);
                trim(passcode);

                // Platzhalter fuer Request, Kontext & Reply.
                // Muss hier lokal definiert werden,
                // da es sonst Probleme mit der Speicherfreigabe gibt.
                Topic request;
                request.set_topic(topic);
                request.set_passcode(passcode);

                // Kontext kann die barbeitung der RPCs beeinflusst werden. Wird nicht genutzt.
                ClientContext context;
                ReturnCode reply;
                Status status = stub_->set_topic(&context, request, &reply);

                // Status / Reply behandeln
                this->handle_status("set_topic()", status, reply);
            } else if (cmd.compare("subscribe") == 0) {
                /* Ueberpruefen, ob Binary des Receivers existiert */
                if (access(receiverExecFile, X_OK) != -1) {
                    /* Receiver starten */
                    if ((rec_pid = fork()) < 0) {
                        std::cerr << "Cannot create process for receiver!\n";
                    } else if (rec_pid == 0) {
                        /* Der Shell-Aufruf */
                        /* xterm -fa 'Monospace' -fs 12 -T Receiver -e ...pub_sub_deliv */
                        /* kann nicht 1:1 uebertragen werden. Bei Aufruf via exec() */
                        /* verhaelt sich das Terminal anders. */
                        /* Alternative: Aufruf von xterm ueber ein Shell-Skript. */
                        /* Allerdings haette man dann 2 Kind-Prozesse. */
                        execl("/usr/bin/xterm", "Receiver", "-fs", "14", receiverExecFile, (char *) NULL);
                        /* -fs 14 wird leider ignoriert! */
                        exit(0); /* Kind beenden */
                    }

                    /* Platzhalter wie oben lokal erstellen ... */

                    SubscriberAddress request;
                    ClientContext context;
                    ReturnCode reply;
                    request.set_address(get_receiver_ip());

                    Status status = stub_->subscribe(&context, request, &reply);

                    this->handle_status("subscribe()", status, reply);
                } else {
                    std::cerr << "Cannot find message receiver executable ("
                              << receiverExecFile << ")!\n";
                    std::cerr << "Press <return> to continue";
                    char c = getc(stdin);
                    continue;
                }
            } else if ((cmd.compare("quit") == 0) ||
                       (cmd.compare("unsubscribe") == 0)) {
                /* Receiver console beenden */
                if (rec_pid > 0) {
                    if (kill(rec_pid, SIGTERM) != 0)
                        std::cerr << "Cannot terminate message receiver!\n";
                    else
                        rec_pid = -1;
                }
                /* Bei quit muss ebenfalls ein unsubscribe() gemacht werden. */


                ClientContext context;
                ReturnCode reply;
                SubscriberAddress request;

                request.set_ip_address(PUBSUB_RECEIVER_IP);
                request.set_port(PUBSUB_RECEIVER_PORT);

                Status status = stub_->publish(&context, request, &reply);

                this->handle_status("unsubscribe()", status, reply);

                /* Shell beenden nur bei quit */
                if (cmd.compare("quit") == 0)
                    break; /* Shell beenden */
            } else  /* kein Kommando -> publish() aufrufen */
            {
                std::string message;
                std::cout << "enter message: > ";
                getline(std::cin, message);
                trim(message);

                Message request;
                request.set_text(message);

                ClientContext context;
                ReturnCode reply;
                Status status = stub_->publish(&context, request, &reply);

                this->handle_status("publish()", status, reply);
            }
        } while (1);
    }

private:
    std::unique_ptr <PubSubService::Stub> stub_;
};

int main(int argc, char **argv) {
    // Einlesen der Argumente. Der Endpunkt des Aufrufs
    // kann über die Option --target eingestellt werden.
    Args args(argc, argv);
    PubSubClient client(grpc::CreateChannel(args.target,grpc::InsecureChannelCredentials()));
    client.run_shell(args);

    return 0;
}
