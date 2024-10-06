#include <array>
#include <functional>


namespace std {
    template <>
    struct hash<std::array<unsigned char, 32>> {
        std::size_t operator()(const std::array<unsigned char, 32>& key) const {
            std::size_t hash_value = 0;
            for (auto byte : key) {
                hash_value ^= std::hash<unsigned char>{}(byte) + 0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
            }
            return hash_value;
        }
    };
}