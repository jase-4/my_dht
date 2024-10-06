#include <util/util.hpp>

namespace MyDHT {
bool extract_quoted_string(const std::string& input, std::string& message) {
    if (input.front() == '"' && input.back() == '"') {
        message = input.substr(1, input.length() - 2);
        return true;
    }
    return false;
}

std::vector<uint8_t> string_to_vector(const std::string& str) {
    return std::vector<uint8_t>(str.begin(), str.end());
}

hash_key hash_of_bytes(const std::vector<uint8_t>& data) {
    hash_key hash{};
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.data(), data.size());
    SHA256_Final(hash.data(), &sha256);
    return hash;
}

file_meta_data create_file_metadata(const std::string& file_path, size_t chunk_size) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) { throw std::runtime_error("Failed to open file: " + file_path); }

    file.seekg(0, std::ios::end);
    size_t total_size = file.tellg();
    file.seekg(0, std::ios::beg);

    file_meta_data metadata;
    metadata.file_name = file_path;
    metadata.total_size = total_size;
    metadata.chunk_size = chunk_size;
    metadata.num_chunks = (total_size + chunk_size - 1) / chunk_size;

    std::vector<uint8_t> buffer(chunk_size);
    size_t bytes_read = 0;

    for (size_t i = 0; i < metadata.num_chunks; ++i) {
        file.read(reinterpret_cast<char*>(buffer.data()), chunk_size);
        bytes_read = file.gcount();
        if (bytes_read > 0) {
            buffer.resize(bytes_read);

            hash_key chunk_hash = hash_of_bytes(buffer);

            metadata.chunk_ids.push_back(chunk_hash);
        }

        buffer.resize(chunk_size);
    }

    return metadata;
}

std::vector<uint8_t> read_chunk_from_file(const std::string& file_path, size_t chunk_index, size_t chunk_size) {
    std::ifstream file(file_path, std::ios::binary);

    if (!file) { throw std::runtime_error("Could not open file for reading chunks"); }

    size_t offset = chunk_index * chunk_size;

    file.seekg(offset);

    if (file.fail()) { throw std::runtime_error("Failed to seek to the correct position in the file"); }

    std::vector<uint8_t> chunk_data;
    chunk_data.resize(chunk_size);

    file.read(reinterpret_cast<char*>(chunk_data.data()), chunk_size);
    size_t bytes_read = file.gcount();

    chunk_data.resize(bytes_read);

    return chunk_data;
}

std::array<uint8_t, 32> hash_of_string(const std::string& data) {
    std::array<uint8_t, 32> hash;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.data(), data.size());
    SHA256_Final(hash.data(), &sha256);
    return hash;
}

uint256_t xor_distance(const uint256_t& id1, const uint256_t& id2) {
    uint256_t result;
    for (size_t i = 0; i < id1.size(); ++i) { result[i] = id1[i] ^ id2[i]; }
    return result;
}

std::vector<uint8_t> string_to_bytes(const std::string& str) {
    std::vector<uint8_t> bytes(str.size() + sizeof(size_t));
    size_t length = str.size();
    std::memcpy(bytes.data(), &length, sizeof(size_t));
    std::memcpy(bytes.data() + sizeof(size_t), str.data(), length);
    return bytes;
}

std::string bytes_to_string(const std::vector<uint8_t>& bytes, size_t& offset) {
    size_t length;
    std::memcpy(&length, bytes.data() + offset, sizeof(size_t));
    offset += sizeof(size_t);
    std::string str(bytes.begin() + offset, bytes.begin() + offset + length);
    offset += length;
    return str;
}

std::vector<uint8_t> size_t_to_bytes(size_t value) {
    std::vector<uint8_t> bytes(sizeof(size_t));
    std::memcpy(bytes.data(), &value, sizeof(size_t));
    return bytes;
}

std::vector<uint8_t> hash_key_to_bytes(const hash_key& key) {
    std::vector<uint8_t> bytes(key.begin(), key.end());
    return bytes;
}

hash_key bytes_to_hash_key(const std::vector<uint8_t>& bytes, size_t& offset) {
    hash_key key;
    std::memcpy(key.data(), bytes.data() + offset, 32);
    offset += 32;
    return key;
}

size_t bytes_to_size_t(const std::vector<uint8_t>& bytes, size_t& offset) {
    size_t value;
    std::memcpy(&value, bytes.data() + offset, sizeof(size_t));
    offset += sizeof(size_t);
    return value;
}

std::vector<uint8_t> vector_of_hash_keys_to_bytes(const std::vector<hash_key>& vec) {
    std::vector<uint8_t> bytes;

    std::vector<uint8_t> num_hash_keys_bytes = size_t_to_bytes(vec.size());
    bytes.insert(bytes.end(), num_hash_keys_bytes.begin(), num_hash_keys_bytes.end());

    for (const auto& key : vec) {
        std::vector<uint8_t> key_bytes = hash_key_to_bytes(key);
        bytes.insert(bytes.end(), key_bytes.begin(), key_bytes.end());
    }

    return bytes;
}

void save_file(const std::string& file_name, const std::vector<uint8_t>& file_data) {
    std::ofstream file(file_name, std::ios::out | std::ios::binary);

    if (!file) { throw std::runtime_error("Could not open file: " + file_name); }

    file.write(reinterpret_cast<const char*>(file_data.data()), file_data.size());

    file.close();

    if (!file) { throw std::runtime_error("Error occurred while writing to file: " + file_name); }
}

std::vector<hash_key> bytes_to_vector_of_hash_keys(const std::vector<uint8_t>& bytes, size_t& offset) {
    std::vector<hash_key> vec;

    size_t num_keys = bytes_to_size_t(bytes, offset);

    for (size_t i = 0; i < num_keys; ++i) { vec.push_back(bytes_to_hash_key(bytes, offset)); }

    return vec;
}

uint256_t xor_arrays(const uint256_t& arr1, const uint256_t& arr2) {
    uint256_t result;
    for (size_t i = 0; i < 32; ++i) { result[i] = arr1[i] ^ arr2[i]; }
    return result;
}

int count_leading_zeros(const uint256_t& xor_value) {
    int leading_zeros = 0;
    for (int i = 0; i < 32; ++i) {
        if (xor_value[i] == 0) {
            leading_zeros += 8;
        } else {
            leading_zeros += __builtin_clz(xor_value[i]) - 24;
            break;
        }
    }
    return leading_zeros;
}

node_info deserialize_node_info(const uint8_t* buffer, size_t& offset) {
    node_info node;

    std::memcpy(node.id.data(), buffer + offset, sizeof(hash_key));
    offset += sizeof(hash_key);
    std::memcpy(&node.address, buffer + offset, sizeof(sockaddr_in));
    offset += sizeof(sockaddr_in);

    return node;
}

void serialize_node_info(const node_info& node, uint8_t* buffer, size_t& offset) {
    std::memcpy(buffer + offset, node.id.data(), sizeof(node.id));
    offset += sizeof(node.id);

    std::memcpy(buffer + offset, &node.address, sizeof(node.address));
    offset += sizeof(node.address);
}

node_list_msg deserialize_node_list_msg(const uint8_t* buffer, size_t& offset) {
    node_list_msg msg;

    if (offset != 1) { offset = 1; }

    std::memcpy(&msg.num_nodes, buffer + offset, sizeof(msg.num_nodes));
    offset += sizeof(msg.num_nodes);

    node_info tnode;
    std::vector<node_info> list2;
    for (int i = 0; i < msg.num_nodes; i++) {
        std::memcpy(&tnode.id, buffer + offset, sizeof(tnode.id));
        offset += sizeof(tnode.id);
        std::memcpy(&tnode.address, buffer + offset, sizeof(tnode.address));
        offset += sizeof(tnode.address);
        list2.emplace_back(tnode);
    }

    msg.node_list = list2;

    return msg;
}

std::vector<uint8_t> serialize_file_data(const file_data& fdata) {
    std::vector<uint8_t> buffer;
    buffer.push_back(static_cast<uint8_t>(fdata.type));
    size_t data_size = fdata.data.size();
    buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&data_size),
                  reinterpret_cast<const uint8_t*>(&data_size + 1));
    buffer.insert(buffer.end(), fdata.data.begin(), fdata.data.end());
    return buffer;
}

std::vector<uint8_t> serialize_store_data_msg(const store_data_msg& msg) {
    std::vector<uint8_t> buffer;

    buffer.push_back(static_cast<uint8_t>(msg.type));
    buffer.insert(buffer.end(), std::begin(msg.key), std::end(msg.key));
    buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&msg.data_size),
                  reinterpret_cast<const uint8_t*>(&msg.data_size + 1));
    std::vector<uint8_t> serialized_data = serialize_file_data(msg.data);
    buffer.insert(buffer.end(), serialized_data.begin(), serialized_data.end());

    return buffer;
}

std::string append_uint256_to_string(const uint256_t& id, const std::string& base_string) {
    std::stringstream ss;

    ss << base_string;

    for (const auto& byte : id) { ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte); }

    return ss.str();
}

file_data deserialize_file_data(const uint8_t* buffer, size_t& offset) {
    file_data fdata;

    fdata.type = static_cast<data_type>(buffer[offset]);
    offset += sizeof(uint8_t);
    std::memcpy(&fdata.files_size, buffer + offset, sizeof(u_int16_t));
    offset += sizeof(size_t);
    fdata.data = std::vector<uint8_t>(buffer + offset, buffer + offset + fdata.files_size);
    offset += fdata.files_size;

    return fdata;
}

store_data_msg deserialize_store_data_msg(const uint8_t* buffer, size_t buffer_size) {
    store_data_msg msg;

    size_t offset = 0;

    offset += sizeof(uint8_t);
    std::memcpy(msg.key.data(), buffer + offset, msg.key.size());
    offset += msg.key.size();
    std::memcpy(&msg.data_size, buffer + offset, sizeof(msg.data_size));
    offset += sizeof(uint16_t);
    msg.data = deserialize_file_data(buffer, offset);

    return msg;
}

void print_array(const std::array<uint8_t, 32>& arr) {
    for (const auto& byte : arr) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    std::cout << std::dec << std::endl;
}

response_msg deserialize_response_msg(const uint8_t* buffer, size_t& offset) {
    response_msg msg;

    std::memcpy(&msg.d_type, buffer + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    std::memcpy(&msg.data_length, buffer + offset, sizeof(msg.data_length));
    offset += sizeof(msg.data_length);

    msg.data = new uint8_t[msg.data_length];

    std::memcpy(msg.data, buffer + offset, msg.data_length);
    offset += msg.data_length;

    return msg;
}

find_msg deserialize_find_msg(const uint8_t* buffer, size_t& offset) {
    find_msg msg;

    std::memcpy(msg.data_hash.data(), buffer + offset, msg.data_hash.size());
    offset += msg.data_hash.size();

    print_array(msg.data_hash);

    std::memcpy(&msg.origin, buffer + offset, sizeof(msg.origin));
    offset += sizeof(msg.origin);

    std::memcpy(&msg.depth, buffer + offset, sizeof(msg.depth));
    offset += sizeof(msg.depth);

    return msg;
}

// fix this
void print_sockaddr_in(struct sockaddr_in* addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr->sin_addr), ip, INET_ADDRSTRLEN);
    printf("IP Address: %s\n", ip);
    printf("Port: %d\n", ntohs(addr->sin_port));
}

join_msg deserialize_join_msg(const uint8_t* buffer, size_t& offset) {
    join_msg msg;
    node_info new_node = deserialize_node_info(buffer, offset);
    msg.new_node = new_node;
    return msg;
}

}  // namespace MyDHT