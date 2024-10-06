#include "util/logger.hpp"

namespace MyDHT {

Logger::Logger() {
    if (!std::filesystem::exists("logs")) { std::filesystem::create_directory("logs"); }
}

void Logger::set_log_file(const uint256_t id) {
    if (!std::filesystem::exists("logs")) { std::filesystem::create_directory("logs"); }
    node_id = id;
    std::string filename = "logs/logfile-";

    filename = append_uint256_to_string(node_id, filename);
    filename += ".log";
    std::cout << "----------------------------------------------------94934939\n"
              << filename << "+++++++++++++++++++++++++++++++++" << std::endl;
    logFile.open(filename, std::ios::trunc);

    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
        std::cout << "------------------------file didnt open\n\n";
    }
}

Logger::~Logger() {
    if (logFile.is_open()) { logFile.close(); }
}

void Logger::info(const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);
    logFile << "[INFO] ID: " << node_id << "\t" << message << std::endl;
}

void Logger::warning(const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);
    logFile << "[WARNING] " << message << std::endl;
}

void Logger::error(const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);
    logFile << "[ERROR] " << message << std::endl;
}

}  // namespace MyDHT
