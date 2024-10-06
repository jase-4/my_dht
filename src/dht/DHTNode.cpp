#include <dht/DHTNode.hpp>

namespace MyDHT {

void DHTNode::run(std::string boot_ip_address, int boot_port) {
    bind_and_listen_node();

    join_network(boot_ip_address, boot_port);

    std::thread requests_thread(&DHTNode::handle_node_requests, this);
    std::thread input_thread(&DHTNode::handle_user_input, this);

    input_thread.join();
    requests_thread.join();
}

void DHTNode::join_network(std::string boot_ip_address, int boot_port) {
    sockaddr_in boot_node;
    boot_node.sin_family = AF_INET;
    boot_node.sin_addr.s_addr = inet_addr(boot_ip_address.c_str());
    boot_node.sin_port = htons(boot_port);

    join_msg join(node);

    send_join(join, socket_fd, boot_node);

    constexpr size_t MAX_BUFFER_SIZE = 1024;

    uint8_t* buffer = new uint8_t[MAX_BUFFER_SIZE];
    sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);

    ssize_t received_length = recvfrom(socket_fd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&src_addr, &addr_len);

    if (received_length > 0) {
        size_t offset = 0;
        message msg;

        msg.type = static_cast<message_type>(buffer[0]);
        offset += sizeof(uint8_t);

        if (msg.type == message_type::NODE_LIST) {
            node_list_msg list = deserialize_node_list_msg(buffer, offset);

            if (list.node_list.size() != 0) {
                for (node_info other_node : list.node_list) {
                    if (other_node.id != node.id) {
                        std::string info = "send_join to ";
                        info = append_uint256_to_string(other_node.id, info);
                        logger.info(info);
                        send_join(join_msg(node), socket_fd, other_node.address);
                    }
                }
                handle_node_list(list);
            }
        }
    }

    delete[] buffer;
}

void DHTNode::bind_and_listen_node() {
    if (bind(socket_fd, (struct sockaddr*)&node.address, sizeof(node.address)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        exit(1);
    }
    std::cout << "Node listening on port " << ntohs(node.address.sin_port) << std::endl;
}

void DHTNode::handle_user_input() {
    std::string input;

    while (true) {
        std::cout << "Enter command (put + \"message\" / get  \"key\" / quit): ";
        std::getline(std::cin, input);

        size_t spacePos = input.find(' ');
        if (spacePos != std::string::npos) {
            std::string command = input.substr(0, spacePos);
            std::string argument = input.substr(spacePos + 1);

            argument.erase(0, argument.find_first_not_of(" \t"));

            if (command == "put") {
                std::string quoted_message;
                if (extract_quoted_string(argument, quoted_message)) {
                    hash_key data_key = hash_of_string(quoted_message);
                    print_array(data_key);

                    file_data data(data_type::STRING, string_to_vector(quoted_message));

                    store_data_init(data_key, data);

                } else {
                    std::cout << "Error: Message should be enclosed in double quotes." << std::endl;
                }
            } else if (command == "upload_file") {
                upload_file(argument);

            } else if (command == "get_file") {
            } else if (command == "get") {
                hash_key key = hash_of_string(argument);
                print_array(key);
                find_msg find(key, node.address, socket_fd);
                find_data(find);

            } else if (command == "exit") {
                exit_node();
                break;
            } else {
                std::cout << "Unknown command: " << command << std::endl;
            }
        } else {
            std::cout << "Error: Input should contain a command and an argument." << std::endl;
        }
    }
}

void DHTNode::find_file(const std::string& file_path) {}

void DHTNode::upload_file(const std::string& file_path) {
    std::unique_lock<std::shared_mutex> lock(map_mutex);
    file_meta_data metadata = create_file_metadata(file_path, 256);

    hash_key file_key = hash_of_string(file_path);
    file_data data(data_type::METADATA, metadata.to_bytes());
    store_data_init(file_key, data);

    for (size_t i = 0; i < metadata.num_chunks; ++i) {
        hash_key chunk_key = metadata.chunk_ids[i];
        std::vector<uint8_t> chunk = read_chunk_from_file(file_path, i, metadata.chunk_size);
        file_data chunk_data(data_type::CHUNK, chunk);
        store_data_init(chunk_key, chunk_data);
    }
}

void DHTNode::exit_node() {
    for (const auto& entry : hash_map) {
        const auto& key = entry.first;
        const auto& data = entry.second;

        node_info closest_node = routing_table.find_closest_other_node(key, node);

        store_data_msg msg(key, data);
        msg.type = message_type::EXIT;
        send_store_data_msg(msg, closest_node.address, socket_fd);
    }
}

void DHTNode::handle_node_requests() {
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
                if (msg.type == message_type::PING) {
                    send_pong(socket_fd, src_addr);
                } else if (msg.type == message_type::FIND) {
                    find_msg find = deserialize_find_msg(buffer, offset);
                    find_data(find);

                } else if (msg.type == message_type::RESPONSE) {
                    response_msg response = deserialize_response_msg(buffer, offset);
                    
                    logger.info("response ");
                    handle_response_msg(response);

                } else if (msg.type == message_type::NODE_LIST) {
                    node_list_msg list = deserialize_node_list_msg(buffer, offset);
                    handle_node_list(list);

                } else if (msg.type == message_type::STORE_DATA) {
                    store_data_msg data = deserialize_store_data_msg(buffer, offset);
                    store_data(data.key, data.data);

                } else if (msg.type == message_type::STORE_DATA_LIST) {
                } else if (msg.type == message_type::JOIN) {
                    join_msg join = deserialize_join_msg(buffer, offset);

                    if ((join.new_node.id != node.id) && (!routing_table.node_exists(join.new_node, node))) {
                        std::string new_info = "This shoudl be in log ";
                        new_info = append_uint256_to_string(join.new_node.id, new_info);
                        logger.info(new_info);

                        routing_table.add_node(join.new_node, node, socket_fd);
                        send_node_list(join.new_node);
                    }

                } else if (msg.type == message_type::EXIT) {
                    store_data_msg data = deserialize_store_data_msg(buffer, offset);
                    store_data(data.key, data.data);

                    logger.info("EXIT WORKED");

                } else if (msg.type == message_type::NOT_FOUND) {
                    std::cout << "\nDATA COULD NOT BE FOUND\n";

                } else {
                    std::cout << "UKNOWN COMMAND";
                }
            });
        }
    }

    delete[] buffer;
}

void DHTNode::find_data(find_msg& find) {
    std::unique_lock<std::shared_mutex> lock(map_mutex);
    if (hash_map.find(find.data_hash) != hash_map.end()) {
        file_data value = hash_map[find.data_hash];
        std::stringstream ss;
        ss << "DATA FOUND " << find.data_hash;
        logger.info(ss.str());

        send_find_response(value, find.origin, socket_fd);
    } else {
        if (find.depth >= MAX_DEPTH) {
            logger.info("DEPTH LIMIT REACHED");
            uint8_t not_found = 7;
            uint8_t* buffer = new uint8_t[sizeof(uint8_t)];
            std::memcpy(buffer, &not_found, sizeof(uint8_t));
            ssize_t sent_length =
                sendto(socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&find.origin, sizeof(find.origin));

            if (sent_length < 0) {
                std::cerr << "Error sending data for depth error" << std::endl;
            } else {
            }

            delete[] buffer;

        } else {
            node_info closent_node = routing_table.find_closest_node(find.data_hash, node);
            find.depth = find.depth + 1;
            send_find_request(find, closent_node.address, socket_fd);
        }
    }
}

void DHTNode::handle_response_msg(const response_msg msg) {
    std::unique_lock<std::mutex> lock(meta_chunk_mutex);

    if (msg.d_type == data_type::STRING) {
        std::string str(reinterpret_cast<char*>(msg.data), msg.data_length);

        std::string message = "Message ";
        message += str;
        std::cout << message << std::endl;

        logger.info(str);
    } else if (msg.d_type == data_type::METADATA) {
        file_meta_data metadata =
            file_meta_data::from_bytes(std::vector<uint8_t>(msg.data, msg.data + msg.data_length));
        hash_key file_key = hash_of_string(metadata.file_name);

        file_tracking_info info;
        info.metadata = metadata;
        info.file_data.resize(metadata.total_size);
        for (const auto& chunk_id : metadata.chunk_ids) { info.missing_chunks.insert(chunk_id); }

        awaiting_files_map[file_key] = info;

        for (const auto& chunk_id : metadata.chunk_ids) {
            find_msg find(chunk_id, node.address, socket_fd);
            find_data(find);
        }
    } else if (msg.d_type == data_type::CHUNK) {
        std::vector<uint8_t> chunk_data(msg.data, msg.data + msg.data_length);
        hash_key chunk_id = hash_of_bytes(chunk_data);

        for (auto& [file_key, info] : awaiting_files_map) {
            auto it = info.missing_chunks.find(chunk_id);
            if (it != info.missing_chunks.end()) {
                size_t chunk_index =
                    std::distance(info.metadata.chunk_ids.begin(),
                                  std::find(info.metadata.chunk_ids.begin(), info.metadata.chunk_ids.end(), chunk_id));

                std::copy(msg.data, msg.data + msg.data_length,
                          info.file_data.begin() + chunk_index * info.metadata.chunk_size);

                info.missing_chunks.erase(it);

                if (info.missing_chunks.empty()) {
                    save_file(info.metadata.file_name, info.file_data);
                    awaiting_files_map.erase(file_key);
                    logger.info("file download");
                }
                break;
            }
        }

    } else {
        logger.error("Invalid d_type fot response_msg");
    }
}

void DHTNode::store_data_init(const hash_key& key, file_data data) {
    int dist = count_leading_zeros(xor_arrays(key, node.id));
    std::vector<node_info> closest_list = routing_table.find_closest_nodes(key, node, 3);
    for (const auto& dest_node : closest_list) {
        if (dest_node.id == node.id) {
            auto result = hash_map.insert({key, data});
            if (result.second) {
                logger.info("DATA STORED in local node");
            } else {
                logger.info("KEY ALREADY EXISTS in local node");
            }
        } else {
            store_data_msg msg(key, data);
            send_store_data_msg(msg, dest_node.address, socket_fd);
            logger.info("Sent data to node: " + append_uint256_to_string(dest_node.id, ""));
        }
    }
}

void DHTNode::store_data(const hash_key& key, file_data data) {
    auto result = hash_map.insert({key, data});
    if (result.second) {
        logger.info("DATA STORED");
    } else {
        logger.info("KEY ALREADY EXISTS");
    }
}

void DHTNode::handle_store_data(int sockfd, sockaddr_in& src_addr) {
    store_data_msg msg = receive_store_data_msg(sockfd, src_addr);
    std::unique_lock<std::shared_mutex> lock(map_mutex);
    auto result = hash_map.insert({msg.key, msg.data});

    if (result.second) {
        logger.info("DATA STORED");
    } else {
        logger.info("KEY ALREADY EXISTS");
    }
}

void DHTNode::send_node_list(const node_info& dest_node) {
    node_list_msg node_list(routing_table.gather_nodes(dest_node.id, 16));
    send_list_to_node(node_list, dest_node.address, socket_fd);
}

void DHTNode::handle_node_list(const node_list_msg& list_of_nodes) {
    routing_table.add_multiple_nodes(list_of_nodes.node_list, node, socket_fd);
}

DHTNode::DHTNode() : thread_pool(5), routing_table(), logger() {};

DHTNode::DHTNode(std::string ip_address, int base_port) : thread_pool(4), routing_table(), logger() {
    generate_node_info(ip_address, base_port);
    logger.set_log_file(node.id);
    routing_table.set_logger(logger);
    std::cout << "node id = " << node.id << std::endl;
    std::cout << "\nDHTNODE IS DONE\n";
}

DHTNode::~DHTNode() {}

void DHTNode::generate_node_info(std::string ip_address, int base_port) {
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    std::cout << "SOCKET FD = " << socket_fd << "\n";

    if (socket_fd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        exit(1);
    }

    memset(&node.address, 0, sizeof(node.address));
    node.address.sin_family = AF_INET;
    node.address.sin_addr.s_addr = inet_addr(ip_address.c_str());
    node.address.sin_port = htons(base_port);
    node.id = generate_256bit_id(node.address.sin_family, node.address.sin_port);

    print_sockaddr_in(&node.address);
}

uint256_t DHTNode::generate_256bit_id(uint32_t binary_ip, uint16_t binary_port) {
    std::vector<uint8_t> input_data(6);
    memcpy(input_data.data(), &binary_ip, sizeof(uint32_t));
    memcpy(input_data.data() + 4, &binary_port, sizeof(uint16_t));

    uint256_t id;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input_data.data(), input_data.size());
    SHA256_Final(id.data(), &sha256);
    return id;
}

}  // namespace MyDHT