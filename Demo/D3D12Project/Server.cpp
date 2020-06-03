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
	running = true;
}

Server::~Server() {
	running = false;
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
	
	WSACleanup();
	consoleOutput = L"Server shutdown\n";
	::OutputDebugString(consoleOutput.c_str());
	//system("pause");

}

void Server::start() {
	while(running){
		fd_set copy = master;
		TIMEVAL tv;
		tv.tv_usec = 10000;

		int socketCount = select(0, &copy, nullptr, nullptr, &tv);

		/*consoleOutput = L"SOCKETCOUNT-------->"+ std::to_wstring(socketCount)+L"\n";
		::OutputDebugString(consoleOutput.c_str());*/

		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];

			if (sock == listening)
			{
				SOCKET client = accept(listening, nullptr, nullptr);

				FD_SET(client, &master);

				// Send a welcome message to the connected client
				/*string welcomeMsg = "Welcome to the Game Server!\r\n";
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);*/
			}
			else 
			{
				char buf[4096];
				ZeroMemory(buf, 4096);

				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0)
				{
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					if (buf[0] == '\\')
					{
						string cmd = string(buf, bytesIn);
						if (cmd == "\\quit")
						{
							running = false;
							break;
						}

						continue;
					}

					/*string msg = string(buf, 0, bytesIn);
					consoleOutput = L"Client (srv)> " + charMsgToWString(msg) + L"\n";
					OutputDebugString(consoleOutput.c_str());*/

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

void Server::sendStartGame() {
	
	for (int i = 0; i < master.fd_count; i++)
	{
		SOCKET sock = master.fd_array[i];

		if (sock != listening)
		{
			char buf[] = "#####";
			const char* msg = buf;
			/*string msgStr = string(buf, 0, sizeof buf);
			consoleOutput = L"SENDING..." + charMsgToWString(msgStr) + L"\n";
			OutputDebugString(consoleOutput.c_str());*/

			send(sock, msg, sizeof buf, 0);
		}
	}
}


wstring Server::charMsgToWString(string& str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}