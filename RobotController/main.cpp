#include "crow_all.h"
#include "../MySocket/MySocket.h"
#include "../PktDef/PktDef.h"
#include <memory>
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>

std::unique_ptr<MySocket> udpSocket = nullptr;
std::string robotIP = "";
int robotPort = 0;
ConnectionType connectionType = ConnectionType::UDP;
int pktCounter = 1;

int main() {
    crow::SimpleApp app;

    // Route to serve index.html
    CROW_ROUTE(app, "/").methods("GET"_method)([]() {
        std::ifstream file("static/index.html");
        if (!file.is_open()) return crow::response(500, "index.html not found.");
        std::stringstream buf;
        buf << file.rdbuf();
        auto res = crow::response(buf.str());
        res.set_header("Content-Type", "text/html");
        return res;
        });

    // Serve CSS and JS
    CROW_ROUTE(app, "/<string>").methods("GET"_method)
        ([](const crow::request&, std::string filename) {
        std::ifstream file("static/" + filename);
        if (!file.is_open()) return crow::response(404);
        std::stringstream buf;
        buf << file.rdbuf();
        crow::response res(buf.str());
        if (filename.find(".css") != std::string::npos)
            res.set_header("Content-Type", "text/css");
        else if (filename.find(".js") != std::string::npos)
            res.set_header("Content-Type", "application/javascript");
        return res;
            });

    // Handle connection to robot
    CROW_ROUTE(app, "/connect").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON");

        robotIP = body["ip"].s();
        robotPort = body["port"].i();
        std::string protocol = body["protocol"].s();
        connectionType = (protocol == "TCP") ? ConnectionType::TCP : ConnectionType::UDP;

        std::cout << "[DEBUG] Connecting to robot at " << robotIP << ":" << robotPort
            << " using " << protocol << std::endl;

        try {
            udpSocket = std::make_unique<MySocket>(
                SocketType::CLIENT,
                robotIP,
                robotPort,
                connectionType,
                1024
            );
            return crow::response(200, "Connected to " + robotIP + ":" + std::to_string(robotPort));
        }
        catch (...) {
            return crow::response(500, "Failed to create socket");
        }
        });

    // Handle drive and sleep commands
    CROW_ROUTE(app, "/telecommand/").methods("PUT"_method)([](const crow::request& req) {
        if (!udpSocket) return crow::response(400, "Not connected.");
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON");

        std::string cmd = body["command"].s();
        int duration = body["duration"].i();
        int speed = body["angle"].i();

        std::cout << "[DEBUG] Sending command '" << cmd << "' to " << robotIP << ":" << robotPort << std::endl;

        PktDef packet;
        packet.SetAck(false);
        packet.SetPktCount(pktCounter++);

        if (cmd == "forward") packet.SetDriveBody(FORWARD, duration, speed);
        else if (cmd == "backward") packet.SetDriveBody(BACKWARD, duration, speed);
        else if (cmd == "left") packet.SetDriveBody(LEFT, duration, speed);
        else if (cmd == "right") packet.SetDriveBody(RIGHT, duration, speed);
        else if (cmd == "sleep") {
            packet.SetCmd(CmdType::SLEEP);
            packet.SetBodyData(nullptr, 0);
        }
        else {
            return crow::response(400, "Unknown command");
        }

        if (cmd != "sleep") packet.SetCmd(CmdType::DRIVE);
        packet.CalcCRC();

        udpSocket->SendData(packet.GenPacket(), packet.GetLength());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        char recvBuf[1024] = {};
        int bytes = udpSocket->GetData(recvBuf);
        if (bytes > 0) {
            PktDef response(recvBuf);
            bool valid = response.CheckCRC(recvBuf, response.GetLength());
            std::string result = "ACK: " + std::string(response.GetAck() ? "Yes" : "No") +
                ", CRC: " + (valid ? "OK" : "Fail");
            return crow::response(200, result);
        }

        return crow::response(200, "Command sent. No response.");
        });

    // Handle telemetry requests
    CROW_ROUTE(app, "/telementry_request/").methods("GET"_method)([]() {
        if (!udpSocket) return crow::response(400, "Not connected.");

        PktDef pkt;
        pkt.SetCmd(CmdType::RESPONSE);
        pkt.SetAck(false);
        pkt.SetPktCount(pktCounter++);
        pkt.SetBodyData(nullptr, 0);
        pkt.CalcCRC();

        udpSocket->SendData(pkt.GenPacket(), pkt.GetLength());
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
        }

        return crow::response(500, "No response from robot.");
        });

    std::cout << "Server running on http://0.0.0.0:18080\n";
    app.port(18080).multithreaded().run();
}