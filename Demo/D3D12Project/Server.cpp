#include "Server.h"

Server::Server() {

	WORD version = MAKEWORD(2, 2);
	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0)
	{
		consoleOutput = L"Can't start Winsock! " + std::to_wstring(wsOk) + L"\n";
		::OutputDebugString(consoleOutput.c_str());
		return;
	}


	listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		consoleOutput = L"Can't create a socket! Quitting\n";
		::OutputDebugString(consoleOutput.c_str());
		return;
	}

	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	listen(listening, SOMAXCONN);

	FD_ZERO(&master);

	FD_SET(listening, &master);

	consoleOutput = L"Server setup complete...\n";
	::OutputDebugString(consoleOutput.c_str());

}

Server::~Server() {

	FD_CLR(listening, &master);
	closesocket(listening);

	string msg = "Server is shutting down. Goodbye\r\n";

	while (master.fd_count > 0)
	{
		SOCKET sock = master.fd_array[0];
		send(sock, msg.c_str(), msg.size() + 1, 0);
		FD_CLR(sock, &master);
		closesocket(sock);
	}
	// Shutdown winsock
	WSACleanup();
	consoleOutput = L"Server shutdown\n";
	::OutputDebugString(consoleOutput.c_str());
	//system("pause");

}

void Server::start() {
	//future<void> futureObj
	//while (futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
	while(true){
		fd_set copy = master;
		TIMEVAL tv;
		tv.tv_usec = 10000;

		// See who's talking to us
		int socketCount = select(0, &copy, nullptr, nullptr, &tv);

		/*consoleOutput = L"SOCKETCOUNT-------->"+ std::to_wstring(socketCount)+L"\n";
		::OutputDebugString(consoleOutput.c_str());*/

		// Loop through all the current connections / potential connect
		for (int i = 0; i < socketCount; i++)
		{
			// Makes things easy for us doing this assignment
			SOCKET sock = copy.fd_array[i];

			// Is it an inbound communication?
			if (sock == listening)
			{
				// Accept a new connection
				SOCKET client = accept(listening, nullptr, nullptr);

				// Add the new connection to the list of connected clients
				FD_SET(client, &master);

				// Send a welcome message to the connected client
				/*string welcomeMsg = "Welcome to the Game Server!\r\n";
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);*/
			}
			else // It's an inbound message
			{
				char buf[4096];
				ZeroMemory(buf, 4096);

				// Receive message
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0)
				{
					// Drop the client
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					// Check to see if it's a command. \quit kills the server
					if (buf[0] == '\\')
					{
						// Is the command quit? 
						string cmd = string(buf, bytesIn);
						if (cmd == "\\quit")
						{
							running = false;
							break;
						}

						// Unknown command
						continue;
					}

					/*string msg = string(buf, 0, bytesIn);
					consoleOutput = L"Client (srv)> " + charMsgToWString(msg) + L"\n";
					OutputDebugString(consoleOutput.c_str());*/

					// Send message to other clients, and definately NOT the listening socket
					for (int i = 0; i < master.fd_count; i++)
					{
						SOCKET outSock = master.fd_array[i];
						if (outSock != listening && outSock != sock)
						{
							/*ostringstream ss;
							ss << buf;
							string strOut = ss.str();*/

							send(outSock, buf, sizeof buf, 0);
						}
					}
				}
			}
		}
	}
}

wstring Server::charMsgToWString(string& str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}