#pragma once
#include <iostream>
#include <string>
#include <thread> 
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include "RenderItem.h"

using namespace std;

class Client
{
public:
    Client();
    Client(const Client& rhs) = delete;
    Client& operator=(const Client& rhs) = delete;
    ~Client();
    void start();
    void sendToServer(float x, float y, float z);
    wstring charMsgToWString(string& str);
    void setPlayer(RenderItem* oth) { otherPlayer = std::move(oth); };

private:
    string ipAddress;			// IP Address of the server
    int port;						// Listening port # on the server
    WSAData data;
    SOCKET sock;
    sockaddr_in hint;
    std::wstring consoleOutput;
    char buf[4096];
    string userInput;
    RenderItem* otherPlayer;
    bool running;
};