#include "PktDef.h"
#include <iostream>
#include <cstring>

// Default constructor
PktDef::PktDef() {
    header.pktCount = 0;
    header.flags = 0;
    header.length = 0;
    data = nullptr;
    crc = 0;
    rawBuffer = nullptr;
}

// Constructs object from raw packet bufferr
PktDef::PktDef(char* rawData) {
    memcpy(&header.pktCount, rawData, 2);
    header.flags = rawData[2];
    header.length = rawData[3];

    int bodyLength = header.length - HEADERSIZE - 1;
    if (bodyLength > 0) {
        data = new char[bodyLength];
        memcpy(data, rawData + 5, bodyLength);
    }
    else {
        data = nullptr;
    }

    crc = rawData[header.length - 1];
    rawBuffer = nullptr;
}

// Sets the command type using bitmask encoding
void PktDef::SetCmd(CmdType cmd) {
    header.flags &= 0xF0; // clear lower 4 bits
    switch (cmd) {
    case CmdType::DRIVE:    header.flags |= 0b00000001; break;
    case CmdType::RESPONSE: header.flags |= 0b00000010; break;
    case CmdType::SLEEP:    header.flags |= 0b00000100; break;
    }
}

// Sets the packet count field
void PktDef::SetPktCount(int count) {
    header.pktCount = count;
}

// Sets the ACK flag
void PktDef::SetAck(bool ack) {
    if (ack)
        header.flags |= 0b00001000;
    else
        header.flags &= 0b11110111;
}

// Populates body with raw data and updates length
void PktDef::SetBodyData(char* inputData, int size) {
    if (data) delete[] data;
    data = new char[size];
    memcpy(data, inputData, size);
    header.length = HEADERSIZE + size + 1; // +1 for CRC
}

// build Drive command body
void PktDef::SetDriveBody(uint8_t dir, uint8_t dur, uint8_t spd) {
    if (data) delete[] data;
    data = new char[3];
    data[0] = dir;
    data[1] = dur;
    data[2] = spd;
    header.length = HEADERSIZE + 3 + 1;
}

// Returns current command type
CmdType PktDef::GetCmd() {
    if (header.flags & 0b00000001) return CmdType::DRIVE;
    if (header.flags & 0b00000010) return CmdType::RESPONSE;
    if (header.flags & 0b00000100) return CmdType::SLEEP;
    return CmdType::DRIVE; // default fallback
}

// Returns whether ACK flag is set
bool PktDef::GetAck() {
    return (header.flags & 0b00001000) != 0;
}

// Getter functions
int PktDef::GetPktCount() { return header.pktCount; }
int PktDef::GetLength() { return header.length; }
char* PktDef::GetBodyData() { return data; }

// Parses DriveBody struct from 3-byte drive command payload
DriveBody PktDef::GetDriveBody() {
    DriveBody d = { 0, 0, 0 };
    if (data && header.length >= HEADERSIZE + 3 + 1) {
        d.direction = data[0];
        d.duration = data[1];
        d.speed = data[2];
    }
    return d;
}

// Parses 9-byte Telemetry structure from packet body
Telemetry PktDef::ParseTelemetry() {
    Telemetry t = { 0 };

    int bodyLength = header.length - HEADERSIZE - 1;
    if (!data || bodyLength < 7) return t;

    // Robot is likely sending big-endian
    t.lastPktCounter = static_cast<uint16_t>((data[0] << 8) | data[1]);
    t.currentGrade = static_cast<uint8_t>(data[2]);
    t.hitCount = static_cast<uint8_t>(data[3]);
    t.lastCmd = static_cast<uint8_t>(data[4]);
    t.lastCmdValue = static_cast<uint8_t>(data[5]);
    t.lastCmdSpeed = static_cast<uint8_t>(data[6]);

    std::cout << "[Telemetry] Parsed:\n"
        << "  Pkt: " << t.lastPktCounter
        << ", Grade: " << (int)t.currentGrade
        << ", Hit: " << (int)t.hitCount
        << ", Cmd: " << (int)t.lastCmd
        << ", Val: " << (int)t.lastCmdValue
        << ", Spd: " << (int)t.lastCmdSpeed << std::endl;

    return t;
}

// Computes CRC by counting all 1-bits across the packet
void PktDef::CalcCRC() {
    uint8_t count = 0;

    // 1. Calculate Header
    uint8_t tempHeader[HEADERSIZE];
    memcpy(tempHeader, &header.pktCount, 2);
    tempHeader[2] = header.flags;
    tempHeader[3] = header.length;

    for (int i = 0; i < HEADERSIZE; ++i) {
        uint8_t b = tempHeader[i];
        while (b) {
            count += b & 1;
            b >>= 1;
        }
    }

    // 2. Calculate Body
    int bodyLength = header.length - HEADERSIZE - 1;
    for (int i = 0; i < bodyLength; ++i) {
        uint8_t b = data[i];
        while (b) {
            count += b & 1;
            b >>= 1;
        }
    }

    crc = count;
}


// Verifies CRC matches computed value for given buffer
bool PktDef::CheckCRC(char* input, int size) {
    uint8_t count = 0;
    for (int i = 0; i < size - 1; i++) {
        uint8_t b = input[i];
        while (b) {
            count += b & 1;
            b >>= 1;
        }
    }
    return count == static_cast<uint8_t>(input[size - 1]);
}

uint8_t PktDef::GetCRC() const {
    return crc;
}

// Serializes packet header + body + CRC into rawBuffer
char* PktDef::GenPacket() {
    if (rawBuffer) delete[] rawBuffer;
    rawBuffer = new char[header.length];

    memcpy(rawBuffer, &header.pktCount, 2);
    rawBuffer[2] = header.flags;
    rawBuffer[3] = header.length;

    int bodyLength = header.length - HEADERSIZE - 1;
    if (bodyLength > 0 && data) {
        memcpy(rawBuffer + 4, data, bodyLength);
    }

    rawBuffer[header.length - 1] = crc;
    return rawBuffer;
}



// Destructor to free allocated memory
PktDef::~PktDef() {
    if (data) delete[] data;
    if (rawBuffer) delete[] rawBuffer;
}