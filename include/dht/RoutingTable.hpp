#pragma once

#include "Kbucket.hpp"

namespace MyDHT {

const int ROUTINGTABLE_SIZE = 64;
class Kbucket;

class RoutingTable {
   private:
    Logger* logger;

    std::vector<Kbucket> buckets;

   public:
    void set_logger(Logger& new_logger);
    std::vector<node_info> find_closest_nodes(const hash_key& target_hash, const node_info& cur_node, size_t k);

    void add_node(const node_info& new_node, const node_info& cur_node, int socket_fd);
    void add_multiple_nodes(const std::vector<node_info>& nodes, const node_info& cur_node, int socket_fd);
    std::vector<node_info> gather_nodes(const uint256_t& requesting_id, int max_nodes);
    node_info find_closest_node(const hash_key& target_hash, const node_info& cur_node);
    bool node_exists(const node_info& new_node, const node_info& cur_node);
    node_info find_closest_other_node(const hash_key& target_hash, const node_info& cur_node);
    RoutingTable();
    ~RoutingTable();
};

}
