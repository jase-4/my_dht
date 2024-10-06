
#pragma once
#include <message/message_structs.hpp>

namespace MyDHT {
bool send_ping(const int& sockfd, int timeout_ms, const sockaddr_in& dest_addr);
void send_find_response(const file_data& data, const sockaddr_in& dest_addr, int sockfd);
void send_list_to_node(const node_list_msg& msg, const sockaddr_in& dest_addr, int sockfd);
bool send_store_data_msg(const store_data_msg& msg, const sockaddr_in& dest_addr, int sockfd);
store_data_msg receive_store_data_msg(int sockfd, sockaddr_in& src_addr);
void send_find_request(const find_msg& find, const sockaddr_in& dest_addr, int sockfd);
void send_pong(int sockfd, const sockaddr_in& origin_addr);
void send_join(join_msg join, int sockfd, sockaddr_in& dest_addr);
} 
