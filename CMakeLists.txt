cmake_minimum_required(VERSION 3.10)
project(MyProject)


# Set the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

# Specify the directories for header files and source files
include_directories(${CMAKE_SOURCE_DIR}/include)

# Collect all the source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Set OpenSSL root directory
set(OPENSSL_ROOT_DIR /opt/homebrew/Cellar/openssl@3/3.3.1)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Define the executables
add_executable(dht_node  src/dht_node.cpp ${SOURCES})


# Link OpenSSL to both targets
target_link_libraries(dht_node OpenSSL::SSL OpenSSL::Crypto)

