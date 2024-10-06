#include <array>
#include <iomanip>
#include <message/message_structs.hpp>
#include <util/util.hpp>

namespace MyDHT {

std::ostream& operator<<(std::ostream& os, const std::array<uint8_t, 32>& arr) {
    for (const auto& byte : arr) { os << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte); }
    os << std::dec;
    return os;
}

std::vector<uint8_t> file_meta_data::to_bytes() const {
    std::vector<uint8_t> bytes;

    std::vector<uint8_t> file_name_bytes = string_to_bytes(file_name);
    bytes.insert(bytes.end(), file_name_bytes.begin(), file_name_bytes.end());
    std::vector<uint8_t> total_size_bytes = size_t_to_bytes(total_size);
    bytes.insert(bytes.end(), total_size_bytes.begin(), total_size_bytes.end());
    std::vector<uint8_t> chunk_size_bytes = size_t_to_bytes(chunk_size);
    bytes.insert(bytes.end(), chunk_size_bytes.begin(), chunk_size_bytes.end());
    std::vector<uint8_t> num_chunks_bytes = size_t_to_bytes(num_chunks);
    bytes.insert(bytes.end(), num_chunks_bytes.begin(), num_chunks_bytes.end());
    std::vector<uint8_t> chunk_ids_bytes = vector_of_hash_keys_to_bytes(chunk_ids);
    bytes.insert(bytes.end(), chunk_ids_bytes.begin(), chunk_ids_bytes.end());

    return bytes;
}

file_meta_data file_meta_data::from_bytes(const std::vector<uint8_t>& bytes) {
    file_meta_data metadata;
    size_t offset = 0;

    metadata.file_name = bytes_to_string(bytes, offset);
    metadata.total_size = bytes_to_size_t(bytes, offset);
    metadata.chunk_size = bytes_to_size_t(bytes, offset);
    metadata.num_chunks = bytes_to_size_t(bytes, offset);
    metadata.chunk_ids = bytes_to_vector_of_hash_keys(bytes, offset);

    return metadata;
}
}  // namespace MyDHT