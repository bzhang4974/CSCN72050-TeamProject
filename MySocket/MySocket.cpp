#include "MySocket.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

// Constructor - sets up socket based on type and protocol
MySocket::MySocket(SocketType type, std::string ip, unsigned int port, ConnectionType connType, unsigned int bufSize)
    : mySocket(type), IPAddr(ip), Port(port), connectionType(connType), MaxSize(bufSize), bTCPConnect(false)
{
    Buffer = new char[MaxSize];

    // Initialize Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Configure address
    memset(&SvrAddr, 0, sizeof(SvrAddr));
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &SvrAddr.sin_addr);

    // Create socket
    int typeVal = (connectionType == ConnectionType::UDP) ? SOCK_DGRAM : SOCK_STREAM;
    int proto = (connectionType == ConnectionType::UDP) ? IPPROTO_UDP : IPPROTO_TCP;

    ConnectionSocket = socket(AF_INET, typeVal, proto);

    if (mySocket == SocketType::SERVER && connectionType == ConnectionType::UDP) {
        // UDP Server: bind to port
        bind(ConnectionSocket, (SOCKADDR*)&SvrAddr, sizeof(SvrAddr));
    }
}

// Destructor
MySocket::~MySocket() {
    closesocket(ConnectionSocket);
    if (mySocket == SocketType::SERVER && connectionType == ConnectionType::TCP)
        closesocket(WelcomeSocket);

    WSACleanup();

    delete[] Buffer;
}

// Sends data to the configured address
void MySocket::SendData(const char* data, int len) {
    if (connectionType == ConnectionType::UDP) {
        sendto(ConnectionSocket, data, len, 0, (SOCKADDR*)&SvrAddr, sizeof(SvrAddr));
    }
    else if (connectionType == ConnectionType::TCP) {
        send(ConnectionSocket, data, len, 0);
    }
}

// Receives data from socket and stores in internal buffer
int MySocket::GetData(char* outBuf) {
    int bytes = 0;
    if (connectionType == ConnectionType::UDP) {
        int addrlen = sizeof(SvrAddr);
        bytes = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (SOCKADDR*)&SvrAddr, &addrlen);
    }
    else {
        bytes = recv(ConnectionSocket, Buffer, MaxSize, 0);
    }

    if (bytes > 0 && outBuf != nullptr) {
        memcpy(outBuf, Buffer, bytes);
    }
    return bytes;
}

// Accessor methods
std::string MySocket::GetIPAddr() { return IPAddr; }
int MySocket::GetPort() { return Port; }
SocketType MySocket::GetType() { return mySocket; }

void MySocket::SetIPAddr(std::string ip) {
    if (!bTCPConnect) IPAddr = ip;
}
void MySocket::SetPort(int p) {
    if (!bTCPConnect) Port = p;
}
void MySocket::SetType(SocketType t) {
    if (!bTCPConnect) mySocket = t;
}

void MySocket::ForceConnected() {
    bTCPConnect = true;
}

// TCP connect/disconnect (not used for UDP)
void MySocket::ConnectTCP() {
    if (connectionType != ConnectionType::TCP || mySocket != SocketType::CLIENT)
        return;

    if (connect(ConnectionSocket, (sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;
        return;
    }

    std::cout << "Connected successfully.\n";
    bTCPConnect = true;
}

void MySocket::DisconnectTCP() {
    if (connectionType == ConnectionType::TCP && bTCPConnect) {
        shutdown(ConnectionSocket, SD_BOTH);
        closesocket(ConnectionSocket);
        ConnectionSocket = INVALID_SOCKET;
        bTCPConnect = false;
    }
}