//
// Created by l.schoeneberg on 30.04.2023.
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <crypto++/sha.h>
#include <crypto++/hex.h>
#include <crypto++/filters.h>

struct HashEntry {
    std::string user;
    std::string hash;
};

std::vector<HashEntry> hash_entries;

std::string generate_sha256_hash(const std::string &input) {
    CryptoPP::SHA256 hash;
    byte digest[CryptoPP::SHA256::DIGESTSIZE];

    hash.CalculateDigest(digest, reinterpret_cast<const byte *>(input.data()), input.size());

    CryptoPP::HexEncoder encoder;
    std::string output;
    CryptoPP::ArraySink as(reinterpret_cast<byte *>(&output[0]), output.size());
    CryptoPP::ArraySource(digest, sizeof(digest), true, new CryptoPP::Redirector(encoder));

    encoder.MessageEnd();
    size_t ready = encoder.MaxRetrievable();
    if (ready)
        output.resize(ready);

    return output;
}

void read_hash_entries() {
    std::ifstream file("hashes.txt");
    if (!file.is_open()) {
        std::cerr << "Cannot open digest 'hashes.txt'.\n";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string user, hash;
        iss >> user >> hash;
        if (!user.empty() && !hash.empty()) {
            hash_entries.push_back({user, hash});
            std::cout << hash_entries.size() - 1 << ": " << user << " " << hash << std::endl;
        } else {
            std::cerr << "Line '" << line << "' malformatted!\n";
        }
    }

    file.close();
    std::cout << "Hash digest 'hashes.txt' read.\n";
}

std::string hash_user_pwd(const std::string &user, const std::string &pwd) {
    std::string input = user + ";" + pwd;
    return generate_sha256_hash(input);
}

int main() {
    read_hash_entries();

    std::string user, pwd;
    std::cout << "Enter user name: ";
    std::getline(std::cin, user);
    std::cout << "Enter password: ";
    std::getline(std::cin, pwd);

    std::string hash_val = hash_user_pwd(user, pwd);
    std::cout << user << " " << hash_val << std::endl;

    return 0;
}
