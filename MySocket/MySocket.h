#pragma once
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int socket_t;
#endif

enum class SocketType { CLIENT, SERVER };
enum class ConnectionType { TCP, UDP };

const int DEFAULT_SIZE = 1024;

class MySocket {
private:
    char* Buffer;
    socket_t WelcomeSocket;
    socket_t ConnectionSocket;
    sockaddr_in SvrAddr;

    SocketType mySocket;
    ConnectionType connectionType;
    std::string IPAddr;
    int Port;
    bool bTCPConnect;
    int MaxSize;

public:
    MySocket(SocketType, std::string, unsigned int, ConnectionType, unsigned int);
    ~MySocket();

    void ConnectTCP();
    void DisconnectTCP();
    void SendData(const char*, int);
    int GetData(char*);

    std::string GetIPAddr();
    void SetIPAddr(std::string);
    void SetPort(int);
    int GetPort();

    SocketType GetType();
    void SetType(SocketType);

    // Extra for testing
    void ForceConnect();
};