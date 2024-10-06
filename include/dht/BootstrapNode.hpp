#pragma once
#include "DHTNode.hpp"

namespace MyDHT {
class BootstrapNode {
   private:
    std::vector<node_info> node_list;
    int socket_fd;
    sockaddr_in address;
    std::mutex node_list_mutex;
    ThreadPool thread_pool;

   public:
    BootstrapNode(std::string ip_address, int base_port);
    ~BootstrapNode();
    void run();
};

}  // namespace MyDHT