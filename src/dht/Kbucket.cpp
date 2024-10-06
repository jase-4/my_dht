#include <dht/Kbucket.hpp>

namespace MyDHT {

bool Kbucket::is_full() const {
    return node_list.size() >= BUCKET_SIZE;
}

void Kbucket::set_logger(Logger& new_logger) {
    logger = &new_logger;
}

void Kbucket::add_node(const node_info& node, int socket_fd) {
    std::unique_lock<std::shared_mutex> lock(bucket_mutex);
    auto it = std::find_if(node_list.begin(), node_list.end(), [&node](const node_info& n) { return n.id == n.id; });

    if (it != node_list.end()) {
        node_list.erase(it);
        node_list.insert(node_list.begin(), node);
        std::string message = "Node Already here moving to front: ";
        message = append_uint256_to_string(node.id, message);
        message += " Successfully inserted";
        logger->info(message);

    } else if (!is_full()) {
        node_list.insert(node_list.begin(), node);
        std::string message = "New Node: ";
        message = append_uint256_to_string(node.id, message);
        message += " Successfully inserted";
        logger->info(message);
    } else {
        node_info& least_recently_seen = node_list.back();
        ping_msg ping;
        if (send_ping(socket_fd, 100, least_recently_seen.address)) {
            std::string message = "Least recently used still active: ";
            message = append_uint256_to_string(least_recently_seen.id, message);
            message += " Discarding new node.";
            logger->info(message);
        } else {
            node_list.pop_back();
            node_list.insert(node_list.begin(), node);
            std::string message = "Least recently not responding. Node popped:  ";
            message = append_uint256_to_string(least_recently_seen.id, message);
            message += " Successfully inserted: ";
            message = append_uint256_to_string(node.id, message);
            logger->info(message);
        }
    }
}

bool Kbucket::node_exists(const node_info& node) {
    std::shared_lock<std::shared_mutex> lock(bucket_mutex);
    return std::find(node_list.begin(), node_list.end(), node) != node_list.end();
}

Kbucket::Kbucket() {}

Kbucket::~Kbucket() {}

}  // namespace MyDHT