#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <unordered_set>
#include <vector>

namespace MyDHT {
using uint256_t = std::array<uint8_t, 32>;
using hash_key = std::array<uint8_t, 32>;

std::ostream& operator<<(std::ostream& os, const std::array<uint8_t, 32>& arr);

enum class data_type : uint8_t {
    FILE,
    STRING,
    METADATA,
    CHUNK,

};

struct file_data {
    data_type type;
    uint16_t files_size;
    std::vector<uint8_t> data;

    file_data() = default;

    file_data(data_type t, const std::vector<uint8_t>& d) : type(t), data(d) { files_size = data.size(); }
};

struct file_meta_data {
    std::string file_name;
    size_t total_size;
    size_t chunk_size;
    size_t num_chunks;
    std::vector<hash_key> chunk_ids;

    std::vector<uint8_t> to_bytes() const;

    static file_meta_data from_bytes(const std::vector<uint8_t>& bytes);
};

struct file_tracking_info {
    file_meta_data metadata;
    std::unordered_set<hash_key> missing_chunks;
    std::vector<uint8_t> file_data;
};

struct node_info {
    uint256_t id;
    struct sockaddr_in address;

    bool operator==(const node_info& other) const {
        return id == other.id && address.sin_addr.s_addr == other.address.sin_addr.s_addr &&
               address.sin_port == other.address.sin_port;
    }
};

const size_t PING_MESSAGE_SIZE = 4;

enum class message_type : uint8_t {
    PING = 0,
    PONG = 1,
    FIND = 2,
    RESPONSE = 3,
    NODE_LIST = 4,
    STORE_DATA = 5,
    STORE_DATA_LIST = 6,
    NOT_FOUND = 7,
    JOIN = 8,
    EXIT = 9
};

struct message {
    message_type type;
};

struct join_msg : public message {
    node_info new_node;

    join_msg() { type = message_type::JOIN; }

    join_msg(node_info node) : message{message_type::JOIN}, new_node(node) {}
};

struct store_data_msg : public message {
    hash_key key;
    uint16_t data_size;
    file_data data;

    store_data_msg() { type = message_type::STORE_DATA; }

    store_data_msg(hash_key d_key, file_data d_data) : message{message_type::STORE_DATA}, key(d_key), data(d_data) {
        data_size = sizeof(data);
    }
};

struct exit_msg : public message {
    hash_key key;
    uint16_t data_size;
    file_data data;

    exit_msg() { type = message_type::EXIT; }

    exit_msg(hash_key d_key, file_data d_data) : message{message_type::EXIT}, key(d_key), data(d_data) {
        data_size = sizeof(data);
    }
};

struct store_data_list_msg : public message {
    uint16_t list_len;
    std::vector<store_data_msg> store_list;

    store_data_list_msg(std::vector<store_data_msg> list) : message{message_type::STORE_DATA_LIST}, store_list(list) {
        list_len = store_list.size();
    }
};

struct ping_msg : public message {
    ping_msg() { type = message_type::PING; }
};

struct pong_msg : public message {
    pong_msg() { type = message_type::PONG; }
};

struct node_list_msg : public message {
    int num_nodes;
    std::vector<node_info> node_list;

    node_list_msg() {
        type = message_type::NODE_LIST;
        num_nodes = 0;
    }

    node_list_msg(const std::vector<node_info> list) : message{message_type::NODE_LIST}, node_list(list) {
        num_nodes = node_list.size();
    }
};

struct find_msg : public message {
    hash_key data_hash;

    sockaddr_in origin;
    uint8_t depth;

    find_msg() : message{message_type::FIND}, depth(0) { origin = sockaddr_in{}; }

    find_msg(const hash_key& hash, const sockaddr_in& addr, const int sock_fd)
        : message{message_type::FIND}, data_hash(hash), origin(addr), depth(0) {}
};

struct response_msg : message {
    data_type d_type;
    uint16_t data_length;
    uint8_t* data;

    response_msg() : message{message_type::RESPONSE}, data_length(0), data(nullptr) {}

    response_msg(data_type type, uint16_t length, const uint8_t* d)
        : message{message_type::RESPONSE}, d_type(type), data_length(length) {
        data = new uint8_t[length];
        std::memcpy(data, d, length);
    }
};

} 