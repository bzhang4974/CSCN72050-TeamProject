#include "pch.h"
#include "CppUnitTest.h"
#include "../PktDef/PktDef.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace PktDefTests
{
    TEST_CLASS(PktDefTests)
    {
    public:

        // ---------- Constructor Tests ----------

        TEST_METHOD(Test01_Default_InitializesToSafeState)
        {
            // Arrange & Act
            PktDef pkt;

            // Assert
            Assert::AreEqual(0, pkt.GetPktCount());
            Assert::AreEqual(0, pkt.GetLength());
            Assert::IsNull(pkt.GetBodyData());
            Assert::IsFalse(pkt.GetAck());
        }

        TEST_METHOD(Test02_RawBuffer_ParsesCorrectly)
        {
            // Arrange
            char buffer[9] = { 0x7B, 0x00, 0x09, 0x09, 0x00, 0x01, 0x05, 0x5A, 0x11 };

            // Act
            PktDef pkt(buffer);

            // Assert
            Assert::AreEqual(123, pkt.GetPktCount());
            Assert::AreEqual(9, pkt.GetLength());
            Assert::IsTrue(pkt.GetAck());
            Assert::IsTrue(pkt.GetCmd() == CmdType::DRIVE);
        }

        // ---------- Setter and Getter Tests ----------

        TEST_METHOD(Test03_SetPktCount_ValidValue)
        {
            // Arrange
            PktDef pkt;

            // Act
            pkt.SetPktCount(256);

            // Assert
            Assert::AreEqual(256, pkt.GetPktCount());
        }

        TEST_METHOD(Test04_SetCmd_Drive_SetsDriveFlagOnly)
        {
            // Arrange
            PktDef pkt;

            // Act
            pkt.SetCmd(CmdType::DRIVE);

            // Assert
            Assert::IsTrue(pkt.GetCmd() == CmdType::DRIVE);
        }

        TEST_METHOD(Test05_SetCmd_Sleep_SetsSleepFlagOnly)
        {
            // Arrange
            PktDef pkt;

            // Act
            pkt.SetCmd(CmdType::SLEEP);

            // Assert
            Assert::IsTrue(pkt.GetCmd() == CmdType::SLEEP);
        }

        TEST_METHOD(Test06_SetCmd_Response_SetsStatusFlagOnly)
        {
            // Arrange
            PktDef pkt;

            // Act
            pkt.SetCmd(CmdType::RESPONSE);

            // Assert
            Assert::IsTrue(pkt.GetCmd() == CmdType::RESPONSE);
        }

        TEST_METHOD(Test07_GetCmd_ReturnsCorrectCommandType)
        {
            // Arrange
            PktDef pkt;
            pkt.SetCmd(CmdType::RESPONSE);

            // Act
            CmdType cmd = pkt.GetCmd();

            // Assert
            Assert::AreEqual((int)CmdType::RESPONSE, (int)cmd);
        }

        TEST_METHOD(Test08_SetBodyData_CorrectSizeAndData_CopiesSuccessfully)
        {
            // Arrange
            PktDef pkt;
            char body[3] = { 1, 5, 90 };

            // Act
            pkt.SetBodyData(body, 3);

            // Assert
            char* data = pkt.GetBodyData();
            Assert::AreEqual((int)body[0], (int)data[0]);
            Assert::AreEqual((int)body[1], (int)data[1]);
            Assert::AreEqual((int)body[2], (int)data[2]);
        }

        TEST_METHOD(Test09_GetBodyData_ReturnsCorrectPointer)
        {
            // Arrange
            PktDef pkt;
            char body[3] = { 3, 2, 100 };
            pkt.SetBodyData(body, 3);

            // Act
            char* result = pkt.GetBodyData();

            // Assert
            Assert::IsNotNull(result);
            Assert::AreEqual(3, (int)result[0]);
        }

        TEST_METHOD(Test10_GetLength_ReturnsCorrectValue)
        {
            // Arrange
            PktDef pkt;
            char body[3] = { 1, 2, 3 };

            // Act
            pkt.SetBodyData(body, 3);

            // Assert
            Assert::AreEqual(9, pkt.GetLength());
        }

        TEST_METHOD(Test11_GetAck_ReturnsTrue_WhenAckIsSet)
        {
            // Arrange
            PktDef pkt;

            // Act
            pkt.SetAck(true);

            // Assert
            Assert::IsTrue(pkt.GetAck());
        }

        // ---------- CRC Validation Tests ----------

        TEST_METHOD(Test12_CheckCRC_CorrectCRC_ReturnsTrue)
        {
            // Arrange
            PktDef pkt;
            pkt.SetCmd(CmdType::DRIVE);
            pkt.SetAck(true);
            pkt.SetPktCount(123);
            pkt.SetDriveBody(1, 5, 90);
            pkt.CalcCRC();

            // Act
            char* buffer = pkt.GenPacket();
            bool isValid = pkt.CheckCRC(buffer, pkt.GetLength());

            // Assert
            Assert::IsTrue(isValid);
        }

        TEST_METHOD(Test13_CheckCRC_IncorrectCRC_ReturnsFalse)
        {
            // Arrange
            PktDef pkt;
            pkt.SetCmd(CmdType::DRIVE);
            pkt.SetAck(true);
            pkt.SetPktCount(123);
            pkt.SetDriveBody(1, 5, 90);
            pkt.CalcCRC();
            char* buffer = pkt.GenPacket();
            buffer[8] = 0x00; // Corrupt the CRC

            // Act
            bool result = pkt.CheckCRC(buffer, pkt.GetLength());

            // Assert
            Assert::IsFalse(result);
        }

        TEST_METHOD(Test14_CalcCRC_ComputesCorrectCRC)
        {
            // Arrange
            PktDef pkt;
            pkt.SetCmd(CmdType::DRIVE);
            pkt.SetAck(true);
            pkt.SetPktCount(123);
            pkt.SetDriveBody(3, 10, 100);
            pkt.CalcCRC();
            char* raw = pkt.GenPacket();

            // Act
            uint8_t expectedCrc = raw[8];
            bool result = pkt.CheckCRC(raw, 9);

            // Assert
            Assert::AreEqual((int)0x11, (int)expectedCrc); // from earlier calc
            Assert::IsTrue(result);
        }

        // ---------- Packet Generation Tests ----------

        TEST_METHOD(Test15_GenPacket_ReturnsNonNull)
        {
            // Arrange
            PktDef pkt;
            pkt.SetCmd(CmdType::DRIVE);
            pkt.SetAck(true);
            pkt.SetPktCount(1);
            pkt.SetDriveBody(1, 5, 90);
            pkt.CalcCRC();

            // Act
            char* buffer = pkt.GenPacket();

            // Assert
            Assert::IsNotNull(buffer);
        }

        TEST_METHOD(Test16_GenPacket_ProducesCorrectSerializedPacket)
        {
            // Arrange
            PktDef pkt;
            pkt.SetCmd(CmdType::DRIVE);
            pkt.SetAck(true);
            pkt.SetPktCount(123);
            pkt.SetDriveBody(1, 5, 90);
            pkt.CalcCRC();

            // Act
            char* raw = pkt.GenPacket();

            // Assert
            Assert::AreEqual((uint8_t)0x7B, (uint8_t)raw[0]); // PktCount LSB
            Assert::AreEqual((uint8_t)0x09, (uint8_t)raw[2]); // Flags = DRIVE + ACK
        }

        // ---------- Edge / Negative Tests ----------

        TEST_METHOD(Test17_SetCmd_InvalidCombination_ThrowsOrIgnores)
        {
            // Arrange
            PktDef pkt;
            pkt.SetCmd(CmdType::DRIVE);
            pkt.SetCmd(CmdType::SLEEP); // overwrite with valid second cmd

            // Act
            CmdType current = pkt.GetCmd();

            // Assert
            Assert::IsTrue(current == CmdType::SLEEP); // Only last should stay
        }

        TEST_METHOD(Test18_SetBodyData_ZeroLength_DoesNotCrash)
        {
            // Arrange
            PktDef pkt;
            char* dummy = new char[0];

            // Act
            pkt.SetBodyData(dummy, 0);

            // Assert
            Assert::AreEqual(6, pkt.GetLength()); // HEADER(5) + CRC(1)
        }

        TEST_METHOD(Test19_Ctor_RawBufferWithBadCRC_SetsSafeState)
        {
            // Arrange
            char raw[9] = { 0x7B, 0x00, 0x09, 0x09, 0x00, 0x01, 0x05, 0x5A, 0x00 }; // Bad CRC

            // Act
            PktDef pkt(raw);
            bool result = pkt.CheckCRC(raw, 9);

            // Assert
            Assert::IsFalse(result);
        }

        // ---------- Optional: Telemetry Packet Test ----------

        TEST_METHOD(Test20_TelemetryPacket_ParsesTelemetryCorrectly)
        {
            // Arrange
            char data[9] = {
                0x2A, 0x00, // LastPktCounter = 42
                0x64, 0x00, // CurrentGrade = 100
                0x01, 0x00, // HitCount = 1
                0x03,       // LastCmd = RIGHT
                0x0A,       // LastCmdValue = 10
                0x5A        // LastCmdSpeed = 90
            };

            PktDef pkt;
            pkt.SetBodyData(data, 9);

            // Act
            Telemetry t = pkt.ParseTelemetry();

            // Assert
            Assert::AreEqual(42, (int)t.lastPktCounter);
            Assert::AreEqual(100, (int)t.currentGrade);
            Assert::AreEqual(1, (int)t.hitCount);
            Assert::AreEqual(3, (int)t.lastCmd);
            Assert::AreEqual(10, (int)t.lastCmdValue);
            Assert::AreEqual(90, (int)t.lastCmdSpeed);
        }
    };
}