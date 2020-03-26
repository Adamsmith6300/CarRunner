#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <future>
#include <string>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")


using namespace std;

class Server
{

public:
    Server();
    Server(const Server& rhs) = delete;
    Server& operator=(const Server& rhs) = delete;
    ~Server();
    void start();
    //void sendToClients(char playerMove);
    wstring charMsgToWString(string& str);

private:
    WSADATA data;
    SOCKET in;
    SOCKET listening;
    sockaddr_in hint;
    std::wstring consoleOutput;
    fd_set master;
    bool running;
};