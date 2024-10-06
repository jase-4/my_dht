#include <dht/BootstrapNode.hpp>
#include <dht/DHTNode.hpp>

using namespace MyDHT;

int main(int argc, char* argv[]) {
    std::string mode = argv[1];
    if (mode == "node") {
        std::string ip_address = argv[2];
        int port = std::stoi(argv[3]);

        std::string boot_ip_address = argv[4];
        int boot_port = std::stoi(argv[5]);

        MyDHT::DHTNode Node(ip_address, port);
        Node.run(boot_ip_address, boot_port);

    } else if (mode == "boot") {
        std::string ip_address = argv[2];
        int port = std::stoi(argv[3]);

        MyDHT::BootstrapNode boot(ip_address, port);
        boot.run();

    } else {
        std::cout << "bad command";
    }
}
