#include <dht/RoutingTable.hpp>
#include <util/util.hpp>

namespace MyDHT {

void RoutingTable::set_logger(Logger& new_logger) {
    logger = &new_logger;
    for (auto& bucket : buckets) { bucket.set_logger(new_logger); }
}

void RoutingTable::add_node(const node_info& new_node, const node_info& cur_node, int socket_fd) {
    uint256_t xor_distance = xor_arrays(cur_node.id, new_node.id);
    int bucket_index = count_leading_zeros(xor_distance);
    if (bucket_index >= 0 && bucket_index < BUCKET_SIZE) {
        buckets[bucket_index].add_node(new_node, socket_fd);

    } else {
        logger->error("bucket index was invalid");
    }
}

void RoutingTable::add_multiple_nodes(const std::vector<node_info>& nodes, const node_info& cur_node, int socket_fd) {
    for (const auto& node : nodes) { add_node(node, cur_node, socket_fd); }
}

std::vector<node_info> RoutingTable::gather_nodes(const uint256_t& requesting_id, int max_nodes) {
    std::vector<node_info> gathered_nodes;

    for (const auto& bucket : buckets) {
        for (const auto& node : bucket.node_list) { gathered_nodes.push_back(node); }
    }

    std::sort(gathered_nodes.begin(), gathered_nodes.end(), [&](const node_info& a, const node_info& b) {
        uint256_t xor_a = xor_arrays(a.id, requesting_id);
        uint256_t xor_b = xor_arrays(b.id, requesting_id);
        return xor_a < xor_b;
    });

    if (gathered_nodes.size() > max_nodes) { gathered_nodes.resize(max_nodes); }

    return gathered_nodes;
}

node_info RoutingTable::find_closest_node(const hash_key& target_hash, const node_info& cur_node) {
    node_info closest_node = cur_node;
    uint256_t smallest_xor_distance = xor_arrays(target_hash, cur_node.id);
    int leading_zeros = count_leading_zeros(smallest_xor_distance);

    for (const auto& bucket : buckets) {
        for (const auto& cur_node : bucket.node_list) {
            uint256_t xor_distance = xor_arrays(cur_node.id, target_hash);
            int current_leading_zeros = count_leading_zeros(xor_distance);

            if (current_leading_zeros >= leading_zeros && xor_distance < smallest_xor_distance) {
                smallest_xor_distance = xor_distance;
                closest_node = cur_node;
            }
        }
    }

    return closest_node;
}

node_info RoutingTable::find_closest_other_node(const hash_key& target_hash, const node_info& cur_node) {
    node_info closest_node;
    uint256_t smallest_xor_distance = std::numeric_limits<uint256_t>::max();
    int leading_zeros = -1;

    for (const auto& bucket : buckets) {
        for (const auto& node : bucket.node_list) {
            if (node.id == cur_node.id) { continue; }

            uint256_t xor_distance = xor_arrays(node.id, target_hash);
            int current_leading_zeros = count_leading_zeros(xor_distance);

            if (current_leading_zeros > leading_zeros ||
                (current_leading_zeros == leading_zeros && xor_distance < smallest_xor_distance)) {
                smallest_xor_distance = xor_distance;
                leading_zeros = current_leading_zeros;
                closest_node = node;
            }
        }
    }

    return closest_node;
}

std::vector<node_info> RoutingTable::find_closest_nodes(const hash_key& target_hash, const node_info& cur_node,
                                                        size_t k) {
    std::vector<node_info> closest_nodes;

    std::vector<std::pair<uint256_t, node_info>> distances;

    uint256_t current_xor_distance = xor_arrays(cur_node.id, target_hash);
    distances.emplace_back(current_xor_distance, cur_node);

    for (const auto& bucket : buckets) {
        for (const auto& node : bucket.node_list) {
            uint256_t xor_distance = xor_arrays(node.id, target_hash);
            distances.emplace_back(xor_distance, node);
        }
    }

    std::sort(distances.begin(), distances.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

    for (size_t i = 0; i < std::min(k, distances.size()); ++i) { closest_nodes.push_back(distances[i].second); }

    return closest_nodes;
}

bool RoutingTable::node_exists(const node_info& new_node, const node_info& cur_node) {
    uint256_t xor_distance = xor_arrays(cur_node.id, new_node.id);
    int bucket_index = count_leading_zeros(xor_distance);
    return buckets[bucket_index].node_exists(new_node);
}

RoutingTable::RoutingTable() {
    std::vector<Kbucket> base_buckets(ROUTINGTABLE_SIZE);
    buckets = std::move(base_buckets);
};
RoutingTable::~RoutingTable() {}

}  // namespace MyDHT