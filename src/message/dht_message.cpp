#include <message/dht_message.hpp>
#include <util/util.hpp>

namespace MyDHT {

void send_find_response(const file_data& data, const sockaddr_in& dest_addr, int sockfd) {
    response_msg response(data.type, data.data.size(), data.data.data());
    size_t offset = 0;

    size_t total_size = sizeof(response_msg) + response.data_length;
    uint8_t* buffer = new uint8_t[total_size];
    std::memcpy(buffer + offset, &response.type, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    std::memcpy(buffer + offset, &response.d_type, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    std::memcpy(buffer + offset, &response.data_length, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    std::memcpy(buffer + offset, response.data, response.data_length);
    offset += response.data_length;

    ssize_t sent_bytes = sendto(sockfd, buffer, total_size, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if (sent_bytes < 0) {
        std::cerr << "Failed to send find response" << std::endl;
        std::cout << "something wroong wrong with response \n";
    }
    delete[] buffer;
}

void send_find_request(const find_msg& find, const sockaddr_in& dest_addr, int sockfd) {
    uint8_t* buffer = new uint8_t[sizeof(find_msg)];
    size_t offset = 0;

    std::memcpy(buffer + offset, &find.type, sizeof(uint8_t));
    offset += sizeof(find.type);
    std::memcpy(buffer + offset, find.data_hash.data(), find.data_hash.size());
    offset += find.data_hash.size();
    std::memcpy(buffer + offset, &find.origin, sizeof(find.origin));
    offset += sizeof(find.origin);
    std::memcpy(buffer + offset, &find.depth, sizeof(find.depth));

    ssize_t sent_bytes = sendto(sockfd, buffer, sizeof(find_msg), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if (sent_bytes < 0) { std::cerr << "Failed to send find response" << std::endl; }
    delete[] buffer;
}

void send_list_to_node(const node_list_msg& msg, const sockaddr_in& dest_addr, int sockfd) {
    size_t total_size = sizeof(uint8_t) + sizeof(uint16_t) + ((sizeof(node_info) * msg.num_nodes));
    uint8_t* buffer = new uint8_t[total_size];
    size_t offset = 0;

    std::memcpy(buffer + offset, &msg.type, sizeof(msg.type));
    offset += sizeof(msg.type);
    std::memcpy(buffer + offset, &msg.num_nodes, sizeof(msg.num_nodes));
    offset += sizeof(msg.num_nodes);

    for (node_info node : msg.node_list) {
        std::memcpy(buffer + offset, &node.id, sizeof(node.id));
        offset += sizeof(node.id);
        std::memcpy(buffer + offset, &node.address, sizeof(node.address));
        offset += sizeof(node.address);
    }

    ssize_t sent_bytes = sendto(sockfd, buffer, offset, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if (sent_bytes < 0) {
    } else {
    }

    delete[] buffer;
}

bool send_ping(const int& sockfd, int timeout_ms, const sockaddr_in& dest_addr) {
    ping_msg ping;
    ssize_t sent_bytes = sendto(sockfd, &ping, sizeof(ping), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if (sent_bytes < 0) {
        std::cerr << "Failed to send ping" << std::endl;
        return false;
    }

    struct timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        std::cerr << "Failed to set socket timeout" << std::endl;
        return false;
    }

    sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);
    pong_msg response;

    ssize_t received_bytes =
        recvfrom(sockfd, &response, sizeof(response), 0, (struct sockaddr*)&sender_addr, &addr_len);
    if (received_bytes < 0) {
        std::cerr << "Failed to receive pong" << std::endl;
        return false;
    }

    char* sender_ip = inet_ntoa(sender_addr.sin_addr);
    unsigned short sender_port = ntohs(sender_addr.sin_port);
    std::cout << "Received pong from IP: " << sender_ip << " Port: " << sender_port << std::endl;

    if (response.type == message_type::PONG) {
        std::cout << "Pong received successfully!" << std::endl;
        return true;
    } else {
        std::cerr << "Unexpected message type received!" << std::endl;
        return false;
    }
}

void send_join(join_msg join, int sockfd, sockaddr_in& dest_addr) {
    int size_buf = sizeof(join_msg);
    uint8_t* buffer = new uint8_t[size_buf];
    size_t offset = 0;

    std::memcpy(buffer + offset, &join.type, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    serialize_node_info(join.new_node, buffer, offset);

    ssize_t sent_bytes = sendto(sockfd, buffer, sizeof(join_msg), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if (sent_bytes < 0) {
    } else {
    }
}

void send_pong(int sockfd, const sockaddr_in& origin_addr) {
    pong_msg response;

    size_t msg_size = sizeof(response);
    ssize_t sent_bytes = sendto(sockfd, &response, msg_size, 0, (struct sockaddr*)&origin_addr, sizeof(origin_addr));

    if (sent_bytes < 0) {
        std::cerr << "Failed to send pong message" << std::endl;
    } else {
        std::cout << "Pong message sent successfully" << std::endl;
    }
}

bool send_store_data_msg(const store_data_msg& msg, const sockaddr_in& dest_addr, int sockfd) {
    std::vector<uint8_t> buffer = serialize_store_data_msg(msg);
    ssize_t sent_bytes =
        sendto(sockfd, buffer.data(), buffer.size(), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    return sent_bytes == buffer.size();
}

store_data_msg receive_store_data_msg(int sockfd, sockaddr_in& src_addr) {
    uint8_t buffer[1024];
    socklen_t addr_len = sizeof(src_addr);

    ssize_t recv_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&src_addr, &addr_len);
    if (recv_bytes <= 0) { std::cerr << "Failed to receive message" << std::endl; }

    return deserialize_store_data_msg(buffer, recv_bytes);
}

}  // namespace MyDHT