#include <dht/BootstrapNode.hpp>

namespace MyDHT {
BootstrapNode::BootstrapNode(std::string ip_address, int base_port) : thread_pool(5) {
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    std::cout << "SOCKET FD = " << socket_fd << "\n";

    if (socket_fd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        exit(1);
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip_address.c_str());
    address.sin_port = htons(base_port);

    if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        close(socket_fd);
        exit(1);
    }
}

BootstrapNode::~BootstrapNode() {}

void BootstrapNode::run() {
    constexpr size_t MAX_BUFFER_SIZE = 1024;
    uint8_t* buffer = new uint8_t[MAX_BUFFER_SIZE];
    sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);

    while (true) {
        ssize_t received_length =
            recvfrom(socket_fd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&src_addr, &addr_len);
        if (received_length > 0) {
            size_t offset = 0;
            message msg;
            msg.type = static_cast<message_type>(buffer[0]);
            offset += sizeof(uint8_t);

            thread_pool.enqueue([this, buffer, received_length, src_addr, offset, msg]() mutable {
                if (msg.type == message_type::JOIN) {
                    node_info new_node = deserialize_node_info(buffer, offset);
                    node_list_msg list_msg;
                    {
                        std::lock_guard<std::mutex> lock(node_list_mutex);
                        list_msg.num_nodes = node_list.size();
                        list_msg.node_list = node_list;
                    }

                    send_list_to_node(list_msg, src_addr, socket_fd);
                    std::cout << "\n";

                    {
                        std::lock_guard<std::mutex> lock(node_list_mutex);
                        node_list.emplace_back(new_node);
                    }

                    {
                        std::lock_guard<std::mutex> lock(node_list_mutex);
                        for (int i = 0; i < node_list.size(); i++) { std::cout << i << " " << node_list[i].id << '\n'; }
                    }
                }
            });
        }
    }

    delete[] buffer;
}
}  // namespace MyDHT