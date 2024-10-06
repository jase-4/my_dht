#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <util/util.hpp>

namespace MyDHT {
class Logger {
   public:
    Logger();
    void set_log_file(uint256_t id);
    ~Logger();
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);

   private:
    uint256_t node_id;
    std::ofstream logFile;
    std::mutex mtx;
};
}  // namespace MyDHT

#endif
