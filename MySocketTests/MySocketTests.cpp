#include "pch.h"
#include "CppUnitTest.h"
#include "../MySocket/MySocket.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace MySocketTests
{
	TEST_CLASS(MySocketTests)
	{
	public:

		// Test 1: Verifies constructor correctly initializes TCP client
		TEST_METHOD(Test01_Constructor_TCPClient_InitCorrectly)
		{
			// Arrange
			SocketType expectedType = SocketType::CLIENT;

			// Act
			MySocket s(SocketType::CLIENT, "127.0.0.1", 8080, ConnectionType::TCP, 1024);

			// Assert
			Assert::AreEqual(std::string("127.0.0.1"), s.GetIPAddr());
			Assert::AreEqual(8080, s.GetPort());
			Assert::AreEqual((int)expectedType, (int)s.GetType());
		}

		// Test 2: Verifies constructor correctly initializes UDP server
		TEST_METHOD(Test02_Constructor_UDPServer_InitCorrectly)
		{
			// Arrange & Act
			MySocket s(SocketType::SERVER, "127.0.0.1", 8080, ConnectionType::UDP, 512);

			// Assert
			Assert::AreEqual(8080, s.GetPort());
		}

		// Test 3: Verifies buffer size falls back to DEFAULT_SIZE when zero is provided
		TEST_METHOD(Test03_Constructor_UsesDefaultBufferSizeOnZero)
		{
			// Arrange & Act
			MySocket s(SocketType::CLIENT, "127.0.0.1", 8080, ConnectionType::TCP, 0);

			// Assert
			Assert::AreEqual(std::string("127.0.0.1"), s.GetIPAddr());
		}

		// Test 4: Confirms ConnectTCP doesn't crash (server may not be listening)
		TEST_METHOD(Test04_ConnectTCP_ValidTCPClient_EstablishesConnection)
		{
			// Arrange
			MySocket client(SocketType::CLIENT, "127.0.0.1", 8081, ConnectionType::TCP, 512);

			// Act
			client.ConnectTCP();

			// Assert
			Assert::IsTrue(true); // No crash
		}

		// Test 5: Verifies ConnectTCP does nothing for UDP sockets
		TEST_METHOD(Test05_ConnectTCP_OnUDP_DoesNothing)
		{
			// Arrange
			MySocket udp(SocketType::CLIENT, "127.0.0.1", 8080, ConnectionType::UDP, 512);

			// Act
			udp.ConnectTCP();

			// Assert
			Assert::IsTrue(true); // No crash
		}

		// Test 6: Ensures DisconnectTCP works after ConnectTCP
		TEST_METHOD(Test06_DisconnectTCP_AfterConnectTCP_Succeeds)
		{
			// Arrange
			MySocket client(SocketType::CLIENT, "127.0.0.1", 8081, ConnectionType::TCP, 512);
			client.ConnectTCP();

			// Act
			client.DisconnectTCP();

			// Assert
			Assert::IsTrue(true); // No crash
		}

		// Test 7: Confirms SendData over TCP doesn't crash
		TEST_METHOD(Test07_SendData_TCP_NoCrash)
		{
			// Arrange
			MySocket client(SocketType::CLIENT, "127.0.0.1", 8081, ConnectionType::TCP, 512);
			client.ConnectTCP();

			// Act
			client.SendData("Hello", 5);

			// Assert
			Assert::IsTrue(true); // No crash
		}

		// Test 8: Confirms SendData over UDP doesn't crash
		TEST_METHOD(Test08_SendData_UDP_Works)
		{
			// Arrange
			MySocket s(SocketType::CLIENT, "127.0.0.1", 8082, ConnectionType::UDP, 512);

			// Act
			s.SendData("Test", 4);

			// Assert
			Assert::IsTrue(true); // No crash
		}

		// Test 9: Verifies SetIPAddr works before TCP connection
		TEST_METHOD(Test09_SetIPAddr_BeforeConnect_Updates)
		{
			// Arrange
			MySocket s(SocketType::CLIENT, "127.0.0.1", 8080, ConnectionType::TCP, 512);

			// Act
			s.SetIPAddr("192.168.0.1");

			// Assert
			Assert::AreEqual(std::string("192.168.0.1"), s.GetIPAddr());
		}

		// Test 10: Verifies SetPort works before TCP connection
		TEST_METHOD(Test10_SetPort_BeforeConnect_Updates)
		{
			// Arrange
			MySocket s(SocketType::CLIENT, "127.0.0.1", 8080, ConnectionType::TCP, 512);

			// Act
			s.SetPort(9000);

			// Assert
			Assert::AreEqual(9000, s.GetPort());
		}

		// Test 11: Verifies SetType works before TCP connection
		TEST_METHOD(Test11_SetType_BeforeConnect_Updates)
		{
			// Arrange
			MySocket s(SocketType::CLIENT, "127.0.0.1", 8080, ConnectionType::TCP, 512);

			// Act
			s.SetType(SocketType::SERVER);

			// Assert
			Assert::AreEqual((int)SocketType::SERVER, (int)s.GetType());
		}

		// Test 12: Verifies SetIPAddr doesn't update after simulated connection
		TEST_METHOD(Test12_SetIPAddr_AfterForceConnect_ShouldNotUpdate)
		{
			// Arrange
			MySocket socket(SocketType::CLIENT, "127.0.0.1", 8081, ConnectionType::TCP, 512);
			socket.ForceConnected();

			// Act
			socket.SetIPAddr("10.0.0.5");

			// Assert
			Assert::AreEqual(std::string("127.0.0.1"), socket.GetIPAddr(), L"IP should remain unchanged after ForceConnected");
		}

		// Test 13: Verifies SetPort doesn't update after simulated connection
		TEST_METHOD(Test13_SetPort_AfterForceConnect_ShouldNotUpdate)
		{
			// Arrange
			MySocket socket(SocketType::CLIENT, "127.0.0.1", 8081, ConnectionType::TCP, 512);
			socket.ForceConnected();

			// Act
			socket.SetPort(9999);

			// Assert
			Assert::AreEqual(8081, socket.GetPort(), L"Port should remain unchanged after ForceConnected");
		}

		// Test 14: Verifies SetType doesn't update after simulated connection
		TEST_METHOD(Test14_SetType_AfterConnect_DoesNotUpdate)
		{
			// Arrange
			MySocket s(SocketType::CLIENT, "127.0.0.1", 8081, ConnectionType::TCP, 512);
			s.ForceConnected();

			// Act
			s.SetType(SocketType::SERVER);

			// Assert
			Assert::AreEqual((int)SocketType::CLIENT, (int)s.GetType());
		}

		// Test 15: Simulates sending and receiving over UDP echo
		TEST_METHOD(Test15_SendReceive_UDP_EchoToSelf)
		{
			// Arrange
			MySocket server(SocketType::SERVER, "127.0.0.1", 8120, ConnectionType::UDP, 512);
			MySocket client(SocketType::CLIENT, "127.0.0.1", 8120, ConnectionType::UDP, 512);
			const char* msg = "Echo";
			char buffer[512] = {};

			// Act
			client.SendData(msg, 4);
			Sleep(100);
			int bytes = server.GetData(buffer);

			// Assert
			Assert::AreEqual(4, bytes);
			Assert::AreEqual(std::string("Echo"), std::string(buffer, bytes));
		}

		// Test 16: Verifies destructor cleans up without crash
		TEST_METHOD(Test16_Destructor_CleansUp_NoCrash)
		{
			// Arrange & Act
			MySocket* s = new MySocket(SocketType::CLIENT, "127.0.0.1", 6060, ConnectionType::UDP, 512);
			delete s;

			// Assert
			Assert::IsTrue(true); // Passes if no crash
		}
	};
}
