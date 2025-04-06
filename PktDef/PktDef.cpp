#include "PktDef.h"
#include <cstring>

PktDef::PktDef() {
    header.pktCount = 0;
    header.flags = 0;
    header.length = 0;
    data = nullptr;
    crc = 0;
    rawBuffer = nullptr;
}

PktDef::PktDef(char* rawData) {
    memcpy(&header.pktCount, rawData, 2);
    header.flags = rawData[2];
    memcpy(&header.length, rawData + 3, 2);

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

void PktDef::SetCmd(CmdType cmd) {
    header.flags &= 0xF0; // clear lower 4 bits
    switch (cmd) {
    case CmdType::DRIVE:    header.flags |= 0b00000001; break;
    case CmdType::RESPONSE: header.flags |= 0b00000010; break;
    case CmdType::SLEEP:    header.flags |= 0b00000100; break;
    }
}

void PktDef::SetPktCount(int count) {
    header.pktCount = count;
}

void PktDef::SetAck(bool val) {
    if (val)
        header.flags |= 0x08;
    else
        header.flags &= ~0x08;
}

void PktDef::SetBodyData(char* inputData, int size) {
    if (data) delete[] data;
    data = new char[size];
    memcpy(data, inputData, size);
    header.length = HEADERSIZE + size + 1; // +1 for CRC
}

void PktDef::SetDriveBody(uint8_t dir, uint8_t dur, uint8_t spd) {
    if (data) delete[] data;
    data = new char[3];
    data[0] = dir;
    data[1] = dur;
    data[2] = spd;
    header.length = HEADERSIZE + 3 + 1;
}

CmdType PktDef::GetCmd() {
    if (header.flags & 0b00000001) return CmdType::DRIVE;
    if (header.flags & 0b00000010) return CmdType::RESPONSE;
    if (header.flags & 0b00000100) return CmdType::SLEEP;
    return CmdType::DRIVE; // default fallback
}

bool PktDef::GetAck() {
    return (header.flags & 0b00001000) != 0;
}

int PktDef::GetPktCount() { return header.pktCount; }
int PktDef::GetLength() { return header.length; }
char* PktDef::GetBodyData() { return data; }

DriveBody PktDef::GetDriveBody() {
    DriveBody d = { 0, 0, 0 };
    if (data && header.length >= HEADERSIZE + 3 + 1) {
        d.direction = data[0];
        d.duration = data[1];
        d.speed = data[2];
    }
    return d;
}

Telemetry PktDef::ParseTelemetry() {
    Telemetry t = { 0 };
    if (data && header.length >= HEADERSIZE + 9 + 1) {
        memcpy(&t.lastPktCounter, data, 2);
        memcpy(&t.currentGrade, data + 2, 2);
        memcpy(&t.hitCount, data + 4, 2);
        t.lastCmd = static_cast<uint8_t>(data[6]);
        t.lastCmdValue = static_cast<uint8_t>(data[7]);
        t.lastCmdSpeed = static_cast<uint8_t>(data[8]);
    }
    return t;
}
void PktDef::CalcCRC() {
    uint8_t count = 0;
    char* temp = GenPacket();
    for (int i = 0; i < header.length - 1; i++) {
        uint8_t b = temp[i];
        while (b) {
            count += b & 1;
            b >>= 1;
        }
    }
    crc = count;
    rawBuffer[header.length - 1] = crc;
}

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

char* PktDef::GenPacket() {
    if (rawBuffer) delete[] rawBuffer;

    rawBuffer = new char[header.length];
    memcpy(rawBuffer, &header.pktCount, 2);
    rawBuffer[2] = header.flags;
    memcpy(rawBuffer + 3, &header.length, 2);

    int bodyLength = header.length - HEADERSIZE - 1;
    if (bodyLength > 0 && data) {
        memcpy(rawBuffer + 5, data, bodyLength);
    }

    rawBuffer[header.length - 1] = crc;
    return rawBuffer;
}

PktDef::~PktDef() {
    if (data) delete[] data;
    if (rawBuffer) delete[] rawBuffer;
}