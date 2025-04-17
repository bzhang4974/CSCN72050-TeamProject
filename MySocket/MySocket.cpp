#include "MySocket.h"
#include <iostream>
#include <cstring>

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

MySocket::MySocket(SocketType type, std::string ip, unsigned int port, ConnectionType connType, unsigned int bufSize)
    : mySocket(type), IPAddr(ip), Port(port), connectionType(connType), MaxSize(bufSize), bTCPConnect(false)
{
    Buffer = new char[MaxSize];

#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    memset(&SvrAddr, 0, sizeof(SvrAddr));
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &SvrAddr.sin_addr);

    int typeVal = (connectionType == ConnectionType::UDP) ? SOCK_DGRAM : SOCK_STREAM;
    int proto = (connectionType == ConnectionType::UDP) ? IPPROTO_UDP : IPPROTO_TCP;

    ConnectionSocket = socket(AF_INET, typeVal, proto);

    if (mySocket == SocketType::SERVER && connectionType == ConnectionType::UDP) {
        bind(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
    }
}

MySocket::~MySocket() {
#ifdef _WIN32
    closesocket(ConnectionSocket);
    if (mySocket == SocketType::SERVER && connectionType == ConnectionType::TCP)
        closesocket(WelcomeSocket);
    WSACleanup();
#else
    close(ConnectionSocket);
    if (mySocket == SocketType::SERVER && connectionType == ConnectionType::TCP)
        close(WelcomeSocket);
#endif

    delete[] Buffer;
}

void MySocket::SendData(const char* data, int len) {
    if (connectionType == ConnectionType::UDP) {
        sendto(ConnectionSocket, data, len, 0, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
    }
    else if (connectionType == ConnectionType::TCP) {
        send(ConnectionSocket, data, len, 0);
    }
}

int MySocket::GetData(char* outBuf) {
    int bytes = 0;
    if (connectionType == ConnectionType::UDP) {
        socklen_t addrlen = sizeof(SvrAddr);
        bytes = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (struct sockaddr*)&SvrAddr, &addrlen);
    }
    else {
        bytes = recv(ConnectionSocket, Buffer, MaxSize, 0);
    }

    if (bytes > 0 && outBuf != nullptr) {
        memcpy(outBuf, Buffer, bytes);
    }
    return bytes;
}

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

// Used in test cases to simulate TCP connection success
void MySocket::ForceConnect() {
    bTCPConnect = true;
}
