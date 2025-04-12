#include "PktDef.h"
#include "MySocket.h"
#include <iostream>
#include <thread>

int main() {
    std::cout << "=== Robot Controller Simulation Start ===\n";

    // Step 1: Create UDP client socket to connect with Robot Simulator
    MySocket sock(SocketType::CLIENT, "127.0.0.1", 5000, ConnectionType::UDP, 1024);

    // Step 2: Create Drive Packet using PktDef
    PktDef packet;
    packet.SetCmd(CmdType::DRIVE);
    packet.SetAck(false);                    // Request ACK
    packet.SetPktCount(1);                  // Packet ID

    // Drive parameters: FORWARD, 5 seconds, speed 90
    packet.SetDriveBody(FORWARD, 5, 90);
    packet.CalcCRC();                         // 
    char* buffer = packet.GenPacket();        // 
    int len = packet.GetLength();


    // Step 3: Send the packet to Robot Simulator
    std::cout << "Sending Drive Packet to Robot Simulator...\n";
    sock.SendData(buffer, len);

    // Optional delay to wait for response
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Step 4: Try to receive response
    char recvBuf[1024] = {};
    int bytes = sock.GetData(recvBuf);

    if (bytes > 0) {
        std::cout << "Received " << bytes << " bytes from Robot Simulator.\n";

        // Try to parse response packet
        PktDef response(recvBuf);
        bool validCRC = response.CheckCRC(recvBuf, response.GetLength());

        std::cout << "Valid CRC: " << (validCRC ? "Yes" : "No") << "\n";
        std::cout << "ACK Received: " << (response.GetAck() ? "Yes" : "No") << "\n";

        if (response.GetCmd() == CmdType::RESPONSE) {
            Telemetry t = response.ParseTelemetry();
            std::cout << "Telemetry Packet Received:\n";
            std::cout << " - LastPktCounter: " << t.lastPktCounter << "\n";
            std::cout << " - CurrentGrade: " << t.currentGrade << "\n";
            std::cout << " - HitCount: " << t.hitCount << "\n";
            std::cout << " - LastCmd: " << static_cast<int>(t.lastCmd) << "\n";
            std::cout << " - LastCmdValue: " << static_cast<int>(t.lastCmdValue) << "\n";
            std::cout << " - LastCmdSpeed: " << static_cast<int>(t.lastCmdSpeed) << "\n";
        }
    }
    else {
        std::cout << "No response received.\n";
    }

    DriveBody dbg = packet.GetDriveBody();
    std::cout << "DEBUG DriveBody:\n";
    std::cout << "Dir = " << (int)dbg.direction << ", Dur = " << (int)dbg.duration << ", Spd = " << (int)dbg.speed << "\n";

    std::cout << "Raw Packet Bytes: ";
    for (int i = 0; i < len; ++i) {
        printf("%02X ", static_cast<unsigned char>(buffer[i]));
    }
    std::cout << "\n";

    std::cout << "=== Controller End ===\n";
    return 0;
}