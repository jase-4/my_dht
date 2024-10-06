# My_DHT

## Description

This project is a robust implementation of a Distributed Hash Table (DHT) designed for efficient data storage and retrieval in a decentralized network. Leveraging the **Kademlia** algorithm for node distribution, the system facilitates peer-to-peer communication, allowing nodes to share and access data without relying on a central server. Key features include:

- **Kademlia-Based Node Management:** Utilizes the Kademlia protocol for efficient node discovery and routing, enabling quick lookup of nodes based on XOR distance.
  
- **Data Handling:** Supports various data types, including strings, files, and metadata, allowing for versatile use cases in distributed applications.
  
- **Chunking Mechanism:** Large files are split into manageable chunks, with each chunk hashed for efficient tracking and retrieval.
  
- **Robust Hashing:** Employs advanced hashing techniques to ensure unique keys for data, minimizing collisions and maximizing retrieval speed.
  
- **Metadata Management:** Each data chunk includes metadata for easy identification and access, ensuring efficient data organization.
  
- **Concurrency Support:** Implemented multi-threading for enhanced performance, allowing multiple operations to occur simultaneously.
  
- **Error Handling and Logging:** Comprehensive logging and error handling to track node behavior and system health.

This DHT implementation is built in **C++** and utilizes a variety of libraries for networking, data serialization, and hashing. It is designed to be scalable and adaptable for various applications in distributed systems.

## Getting Started

### Prerequisites

- [CMake](https://cmake.org/download/) for building the project
- A C++ compiler (e.g., g++, clang++)

### Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/jase-4/my_dht.git
