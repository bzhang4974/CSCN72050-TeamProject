//#include "../PktDef/PktDef.h"
//#include "../MySocket/MySocket.h"
//#include <iostream>
//#include <thread>
//
//int main() {
//    std::cout << "=== Robot Controller Simulation Start ===\n";
//
//    // Step 1: Create UDP client socket to connect with Robot Simulator
//    MySocket sock(SocketType::CLIENT, "127.0.0.1", 5000, ConnectionType::UDP, 1024);
//
//    // Step 2: Create Drive Packet using PktDef
//    PktDef packet;
//    packet.SetCmd(CmdType::DRIVE);
//    packet.SetAck(false);                    // Request ACK
//    packet.SetPktCount(1);                  // Packet ID
//
//    // Drive parameters: FORWARD, 5 seconds, speed 90
//    packet.SetDriveBody(FORWARD, 5, 80);
//    packet.CalcCRC();                         // 
//    char* buffer = packet.GenPacket();        // 
//    int len = packet.GetLength();
//
//
//    // Step 3: Send the packet to Robot Simulator
//    std::cout << "Sending Drive Packet to Robot Simulator...\n";
//    sock.SendData(buffer, len);
//
//    // Optional delay to wait for response
//    std::this_thread::sleep_for(std::chrono::milliseconds(100));
//
//    // Step 4: Try to receive response
//    char recvBuf[1024] = {};
//    int bytes = sock.GetData(recvBuf);
//
//    if (bytes > 0) {
//        std::cout << "Received " << bytes << " bytes from Robot Simulator.\n";
//
//        // Try to parse response packet
//        PktDef response(recvBuf);
//        bool validCRC = response.CheckCRC(recvBuf, response.GetLength());
//
//        std::cout << "Valid CRC: " << (validCRC ? "Yes" : "No") << "\n";
//        std::cout << "ACK Received: " << (response.GetAck() ? "Yes" : "No") << "\n";
//
//        if (response.GetCmd() == CmdType::RESPONSE) {
//            Telemetry t = response.ParseTelemetry();
//            std::cout << "Telemetry Packet Received:\n";
//            std::cout << " - LastPktCounter: " << t.lastPktCounter << "\n";
//            std::cout << " - CurrentGrade: " << t.currentGrade << "\n";
//            std::cout << " - HitCount: " << t.hitCount << "\n";
//            std::cout << " - LastCmd: " << static_cast<int>(t.lastCmd) << "\n";
//            std::cout << " - LastCmdValue: " << static_cast<int>(t.lastCmdValue) << "\n";
//            std::cout << " - LastCmdSpeed: " << static_cast<int>(t.lastCmdSpeed) << "\n";
//        }
//    }
//    else {
//        std::cout << "No response received.\n";
//    }
//
//    DriveBody dbg = packet.GetDriveBody();
//    std::cout << "DEBUG DriveBody:\n";
//    std::cout << "Dir = " << (int)dbg.direction << ", Dur = " << (int)dbg.duration << ", Spd = " << (int)dbg.speed << "\n";
//
//    std::cout << "Raw Packet Bytes: ";
//    for (int i = 0; i < len; ++i) {
//        printf("%02X ", static_cast<unsigned char>(buffer[i]));
//    }
//    std::cout << "\n";
//
//    std::cout << "=== Controller End ===\n";
//    return 0;
//}

#include "crow_all.h"
#include "../MySocket/MySocket.h"
#include "../PktDef/PktDef.h"
#include <memory>
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>

// Global state
std::unique_ptr<MySocket> udpSocket = nullptr;
std::string robotIP = "";
int robotPort = 0;
int pktCounter = 1; // unique ID for each packet

int main() {
    crow::SimpleApp app;

    // Route: Serve GUI
    CROW_ROUTE(app, "/").methods("GET"_method)([]() {
        std::ifstream file("static/index.html");
        if (!file.is_open()) return crow::response(500, "index.html not found.");
        std::stringstream buf; buf << file.rdbuf();
        return crow::response(buf.str());
        });

    // Route: Connect
    CROW_ROUTE(app, "/connect").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON");

        robotIP = body["ip"].s();
        robotPort = body["port"].i();

        std::cout << "[DEBUG] Connecting to robot at " << robotIP << ":" << robotPort << std::endl;

        try {
            udpSocket = std::make_unique<MySocket>(SocketType::CLIENT, robotIP, robotPort, ConnectionType::UDP, 1024);
            return crow::response(200, "Connected to " + robotIP + ":" + std::to_string(robotPort));
        }
        catch (...) {
            return crow::response(500, "Failed to create socket");
        }
        });

    // Route: Telecommand
    CROW_ROUTE(app, "/telecommand/").methods("PUT"_method)([](const crow::request& req) {
        if (!udpSocket) return crow::response(400, "Not connected.");

        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON");

        std::string cmd = body["command"].s();
        int duration = body["duration"].i();
        int speed = body["angle"].i(); // reuse as speed

        std::cout << "[DEBUG] Sending command '" << cmd << "' to " << robotIP << ":" << robotPort << std::endl;

        PktDef packet;
        packet.SetAck(false);
        packet.SetPktCount(pktCounter++);

        if (cmd == "forward")
            packet.SetDriveBody(FORWARD, duration, speed);
        else if (cmd == "backward")
            packet.SetDriveBody(BACKWARD, duration, speed);
        else if (cmd == "left")
            packet.SetDriveBody(LEFT, duration, speed);
        else if (cmd == "right")
            packet.SetDriveBody(RIGHT, duration, speed);
        else if (cmd == "sleep") {
            packet.SetCmd(CmdType::SLEEP);
            packet.SetBodyData(nullptr, 0);
        }
        else {
            return crow::response(400, "Unknown command: " + cmd);
        }

        if (cmd != "sleep") packet.SetCmd(CmdType::DRIVE);
        packet.CalcCRC();
        char* buf = packet.GenPacket();
        int len = packet.GetLength();

        udpSocket->SendData(buf, len);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Try to read response
        char recvBuf[1024] = {};
        int bytes = udpSocket->GetData(recvBuf);
        if (bytes > 0) {
            std::cout << "[DEBUG] Received response of " << bytes << " bytes from robot." << std::endl;
            PktDef response(recvBuf);
            bool valid = response.CheckCRC(recvBuf, response.GetLength());
            std::string result = "ACK: " + std::string(response.GetAck() ? "Yes" : "No") +
                ", CRC: " + (valid ? "OK" : "Fail");
            return crow::response(200, result);
        }
        else {
            std::cout << "[DEBUG] No response received from robot." << std::endl;
        }

        return crow::response(200, "Command sent. No response.");
        });

    // Route: Telemetry
    CROW_ROUTE(app, "/telementry_request/").methods("GET"_method)([]() {
        if (!udpSocket) return crow::response(400, "Not connected.");

        PktDef pkt;
        pkt.SetCmd(CmdType::RESPONSE);
        pkt.SetAck(false);
        pkt.SetPktCount(pktCounter++);
        pkt.SetBodyData(nullptr, 0);
        pkt.CalcCRC();

        char* buf = pkt.GenPacket();
        int len = pkt.GetLength();
        udpSocket->SendData(buf, len);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        char recvBuf[1024] = {};
        int bytes = udpSocket->GetData(recvBuf);
        if (bytes > 0) {
            PktDef res(recvBuf);
            if (res.GetCmd() == CmdType::RESPONSE) {
                Telemetry t = res.ParseTelemetry();
                std::ostringstream oss;
                oss << "LastPkt: " << t.lastPktCounter << "\n";
                oss << "Grade: " << t.currentGrade << "\n";
                oss << "HitCount: " << t.hitCount << "\n";
                oss << "LastCmd: " << static_cast<int>(t.lastCmd) << "\n";
                oss << "LastValue: " << static_cast<int>(t.lastCmdValue) << "\n";
                oss << "Speed: " << static_cast<int>(t.lastCmdSpeed) << "\n";
                return crow::response(200, oss.str());
            }
            else {
                return crow::response(200, "Received non-telemetry packet.");
            }
        }

        return crow::response(500, "No response from robot.");
        });

    std::cout << "Server running on http://0.0.0.0:18080\n";
    app.port(18080).multithreaded().run();
}