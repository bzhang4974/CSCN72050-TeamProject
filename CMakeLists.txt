cmake_minimum_required(VERSION 3.10)
project(RobotController)

set(CMAKE_CXX_STANDARD 17)

# Include all relevant folders
include_directories(
    ${PROJECT_SOURCE_DIR}/MySocket
    ${PROJECT_SOURCE_DIR}/PktDef
    ${PROJECT_SOURCE_DIR}/External/Crow
)

# Add source files
add_executable(RobotController
    RobotController/main.cpp
    MySocket/MySocket.cpp
    PktDef/PktDef.cpp
)

# Find and link dependencies
find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(RobotController
    OpenSSL::SSL
    OpenSSL::Crypto
)