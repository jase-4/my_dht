#pragma once

#include <algorithm>
#include <message/message_structs.hpp>
#include <mutex>
#include <shared_mutex>
#include <util/logger.hpp>
#include <util/util.hpp>

namespace MyDHT {

const int BUCKET_SIZE = 20;

class Kbucket {
   private:
    Logger *logger;
    std::shared_mutex bucket_mutex;

   public:
    std::vector<node_info> node_list;
    Kbucket(/* args */);
    ~Kbucket();
    void set_logger(Logger &new_logger);
    bool is_full() const;
    void add_node(const node_info &node, int socket_fd);
    bool node_exists(const node_info &node);
};

}  // namespace MyDHT