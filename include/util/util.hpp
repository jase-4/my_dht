#pragma once
#include <openssl/sha.h>

#include <iostream>
#include <message/dht_message.hpp>
#include <message/message_structs.hpp>
#include <sstream>
#include <util/logger.hpp>

namespace MyDHT {

bool extract_quoted_string(const std::string& input, std::string& message);
std::vector<uint8_t> read_chunk_from_file(const std::string& file_path, size_t chunk_index, size_t chunk_size);
file_meta_data create_file_metadata(const std::string& file_path, size_t chunk_size);
std::vector<uint8_t> string_to_vector(const std::string& str);
std::string append_uint256_to_string(const uint256_t& id, const std::string& base_string);
std::array<uint8_t, 32> hash_of_string(const std::string& data);
hash_key hash_of_bytes(const std::vector<uint8_t>& data);
uint256_t xor_arrays(const uint256_t& arr1, const uint256_t& arr2);
int count_leading_zeros(const uint256_t& xor_value);
std::string bytes_to_string(const std::vector<uint8_t>& bytes, size_t& offset);
std::vector<uint8_t> string_to_bytes(const std::string& str);

void serialize_node_info(const node_info& node, uint8_t* buffer, size_t& offset);
node_info deserialize_node_info(const uint8_t* buffer, size_t& offset);
node_list_msg deserialize_node_list_msg(const uint8_t* buffer, size_t& offset);
std::vector<uint8_t> serialize_file_data(const file_data& fdata);
std::vector<uint8_t> serialize_store_data_msg(const store_data_msg& msg);
file_data deserialize_file_data(const uint8_t* buffer, size_t& offset);
store_data_msg deserialize_store_data_msg(const uint8_t* buffer, size_t buffer_size);

void print_array(const std::array<uint8_t, 32>& arr);
response_msg deserialize_response_msg(const uint8_t* buffer, size_t& offset);
find_msg deserialize_find_msg(const uint8_t* buffer, size_t& offset);
void print_sockaddr_in(struct sockaddr_in* addr);
join_msg deserialize_join_msg(const uint8_t* buffer, size_t& offset);
std::vector<uint8_t> size_t_to_bytes(size_t value);
size_t bytes_to_size_t(const std::vector<uint8_t>& bytes, size_t& offset);
std::vector<uint8_t> vector_of_hash_keys_to_bytes(const std::vector<hash_key>& vec);
std::vector<hash_key> bytes_to_vector_of_hash_keys(const std::vector<uint8_t>& bytes, size_t& offset);
std::vector<uint8_t> hash_key_to_bytes(const hash_key& key);
hash_key bytes_to_hash_key(const std::vector<uint8_t>& bytes, size_t& offset);
void save_file(const std::string& file_name, const std::vector<uint8_t>& file_data);
} 
