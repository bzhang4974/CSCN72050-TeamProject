#pragma once
#include <cstdint>

enum class CmdType { DRIVE, SLEEP, RESPONSE };

const int FORWARD = 1;
const int BACKWARD = 2;
const int RIGHT = 3;
const int LEFT = 4;
const int HEADERSIZE = 5; // PktCount(2) + Flags(1) + Length(2)

class PktDef {
private:
    struct Header {
        uint16_t pktCount;
        uint8_t flags;
        uint16_t length;
    };

    struct DriveBody {
        uint8_t direction;
        uint8_t duration;
        uint8_t speed;
    };

    Header header;
    char* data;
    uint8_t crc;
    char* rawBuffer;

public:
    PktDef();
    PktDef(char* rawData); // Parse from raw data

    void SetCmd(CmdType cmd);
    void SetPktCount(int count);
    void SetBodyData(char* inputData, int size);

    CmdType GetCmd();
    bool GetAck();
    int GetPktCount();
    int GetLength();
    char* GetBodyData();

    void CalcCRC();
    bool CheckCRC(char* input, int size);
    char* GenPacket(); // Serialize packet into rawBuffer

    ~PktDef();
};