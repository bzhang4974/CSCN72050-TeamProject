#pragma once
#include <cstdint>

enum class CmdType { DRIVE, SLEEP, RESPONSE };

const int FORWARD = 1;
const int BACKWARD = 2;
const int RIGHT = 3;
const int LEFT = 4;
const int HEADERSIZE = 4; // PktCount(2) + Flags(1) + Length(1)

struct DriveBody {
    uint8_t direction;
    uint8_t duration;
    uint8_t speed;
};

struct Telemetry {
    uint16_t lastPktCounter;
    uint16_t currentGrade;
    uint16_t hitCount;
    uint8_t lastCmd;
    uint8_t lastCmdValue;
    uint8_t lastCmdSpeed;
};

class PktDef {
private:
    struct Header {
        uint16_t pktCount;
        uint8_t flags;
        uint8_t length;
    };

    Header header;
    char* data;
    uint8_t crc;
    char* rawBuffer;

public:
    // Default constructor initializes an empty packet
    PktDef();

    
    PktDef(char* rawData);
    ~PktDef();

    // Sets the command type (DRIVE, RESPONSE, SLEEP) in the flag field
    void SetCmd(CmdType cmd);

    // Sets the packet count value
    void SetPktCount(int count);

    // Sets the ACK flag ON or OFF
    void SetAck(bool val);

    // Sets raw body data and updates packet length (for custom payloads)
    void SetBodyData(char* inputData, int size);

    // Sets Drive command body (direction, duration, speed) and updates length
    void SetDriveBody(uint8_t dir, uint8_t dur, uint8_t spd);

    // Returns the command type based on flag bits
    CmdType GetCmd();

    // Returns true if ACK flag is set
    bool GetAck();

    int GetPktCount();

    int GetLength();

    // Returns a pointer to the packet body data
    char* GetBodyData();

    uint8_t GetCRC() const;

    // Returns the parsed DriveBody struct (3-byte drive command)
    DriveBody GetDriveBody();

    // Returns the parsed Telemetry struct (9-byte status response)
    Telemetry ParseTelemetry();

    void CalcCRC();

    bool CheckCRC(char* input, int size);

    // Serializes the packet into rawBuffer and returns it
    char* GenPacket();
};