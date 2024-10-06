#pragma once
#include <random>
#include <string>
#include <unordered_map>
#include <unistd.h>
#include <netinet/in.h>
#include <util/ThreadPool.hpp>
#include <util/special_hash.hpp>
#include <message/dht_message.hpp>
#include "RoutingTable.hpp"

namespace MyDHT {
const int MAX_DEPTH = 5;
class DHTNode {
   private:
    int socket_fd;
    std::unordered_map<hash_key, file_data> hash_map;
    std::unordered_map<hash_key, file_tracking_info> awaiting_files_map;
    std::shared_mutex map_mutex;
    std::mutex meta_chunk_mutex;
    ThreadPool thread_pool;
    void generate_node_info(std::string ip_address, int base_port);
    void bind_and_listen_node();
    hash_key generate_256bit_id(uint32_t binary_ip, uint16_t binary_port);
    void handle_user_input();
    void handle_node_requests();
    void store_data_init(const hash_key &key, file_data data);
    void find_data(find_msg &find);
    void send_node_list(const node_info &dest_node);
    void handle_node_list(const node_list_msg &list_of_nodes);
    void store_data(const hash_key &key, file_data data);
    void handle_store_data(int sockfd, sockaddr_in &src_addr);
    void handle_response_msg(const response_msg msg);
    void join_network(std::string boot_ip_address, int boot_port);
    void upload_file(const std::string &file_path);
    void exit_node();
    void find_file(const std::string &file_path);

   public:
    Logger logger;
    DHTNode();
    DHTNode(std::string ip_address, int base_port);
    ~DHTNode();

    void run(std::string boot_ip_address, int boot_port);
    RoutingTable routing_table;
    node_info node;
};

}  // namespace MyDHT
