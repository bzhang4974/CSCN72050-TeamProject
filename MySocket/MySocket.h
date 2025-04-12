#pragma once
#include <string>
#include <winsock2.h>

// Enum for socket type
enum class SocketType { CLIENT, SERVER };

// Enum for connection type
enum class ConnectionType { TCP, UDP };

const int DEFAULT_SIZE = 1024;

class MySocket {
private:
    char* Buffer;                   // Data buffer
    SOCKET WelcomeSocket;          // TCP Server only
    SOCKET ConnectionSocket;       // Main socket used
    sockaddr_in SvrAddr;           // Server or destination address

    SocketType mySocket;           // CLIENT or SERVER
    ConnectionType connectionType; // TCP or UDP
    std::string IPAddr;            // IP Address
    int Port;                      // Port number
    bool bTCPConnect;              // TCP connection status
    int MaxSize;                   // Buffer size

public:
    // Constructor & Destructor
    MySocket(SocketType, std::string, unsigned int, ConnectionType, unsigned int);
    ~MySocket();

    // Communication methods
    void ConnectTCP();
    void DisconnectTCP();

    void SendData(const char*, int);
    int GetData(char*);

    // Setters and Getters
    std::string GetIPAddr();
    void SetIPAddr(std::string);
    void SetPort(int);
    int GetPort();

    SocketType GetType();
    void SetType(SocketType);
};
