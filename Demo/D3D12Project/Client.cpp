#include "Client.h"

Client::Client() {
	ipAddress = "127.0.0.1";
	port = 54000;
	// Initialize WinSock
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		consoleOutput = L"Can't start Winsock, Err #" + to_wstring(wsResult) + L"\n";
		::OutputDebugString(consoleOutput.c_str());
		return;
	}


	// Create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		consoleOutput = L"Can't create socket, Err #" + to_wstring(WSAGetLastError()) + L"\n";
		::OutputDebugString(consoleOutput.c_str());
		WSACleanup();
		return;
	}

	// Fill in a hint structure
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	// Connect to server
	int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		consoleOutput = L"Can't connect to server, Err #" + to_wstring(WSAGetLastError()) + L"\n";
		::OutputDebugString(consoleOutput.c_str());
		closesocket(sock);
		WSACleanup();
		return;
	}
	consoleOutput = L"Connected to server!\n";
	::OutputDebugString(consoleOutput.c_str());
	running = true;
}

Client::~Client() {
	running = false;
	// Gracefully close down everything
	closesocket(sock);
	WSACleanup();
}

void Client::start() {
	//future<void> futureObj
	//while (futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
	while(running){
		ZeroMemory(buf, 4096);
		int bytesReceived = recv(sock, buf, 4096, 0);
		if (bytesReceived > 0)
		{
			// Echo response to console
			string msg = string(buf, 0, bytesReceived);
			/*consoleOutput = L"SERVER> " + charMsgToWString(msg) + L"\n";
			OutputDebugString(consoleOutput.c_str());*/

			std::vector<string> bufferPos;
			string word = "";
			for (auto x : msg)
			{
				if (x == ' ')
				{
					bufferPos.push_back(word);
					word = "";
				}
				else
				{
					word = word + x;
				}
			}

			float x = ::atof(bufferPos[0].c_str());
			float y = ::atof(bufferPos[1].c_str());
			float z = ::atof(bufferPos[2].c_str());

			/*consoleOutput = L"Pos.X" + to_wstring(x) + L"\n";
			OutputDebugString(consoleOutput.c_str());
			consoleOutput = L"Pos.Y" + to_wstring(y) + L"\n";
			OutputDebugString(consoleOutput.c_str());
			consoleOutput = L"Pos.Z" + to_wstring(z) + L"\n";
			OutputDebugString(consoleOutput.c_str());*/


			XMMATRIX boxRotate = XMMatrixRotationY(0.5f * MathHelper::Pi);
			XMMATRIX boxScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
			XMMATRIX boxOffset = XMMatrixTranslation(x, y, z);
			XMMATRIX boxWorld = boxRotate * boxScale * boxOffset;
			otherPlayer->Geo;
			XMStoreFloat4x4(&otherPlayer->World, boxWorld);
			////calculate new bounding box of first box
			//calcAABB(boxBoundingVertPosArray, otherPlayer->World, otherPlayer->boundingboxminvertex, otherPlayer->boundingboxmaxvertex);
			////formerly mboxritemmovable
			//otherPlayer->NumFramesDirty++;
		}
	}
}

wstring Client::charMsgToWString(string& str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

void Client::sendToServer(float x, float y, float z) {
	string consoleBuffer;

	char bufferX[128];
	int ret = snprintf(bufferX, sizeof bufferX, "%f ", x);
	/*consoleBuffer = string(bufferX, 0, ret);
	consoleOutput = L"Pos.X" + charMsgToWString(consoleBuffer) + L"\n";
	OutputDebugString(consoleOutput.c_str());*/

	if (ret < 0) {
		return;
	}
	if (ret >= sizeof bufferX) {
		// Result was truncated - resize the buffer and retry.
		consoleOutput = L"pos.x buffer too small\n";
	}

	char bufferY[128];
	ret = snprintf(bufferY, sizeof bufferY, "%f ", y);
	/*consoleBuffer = string(bufferY, 0, ret);
	consoleOutput = L"Pos.Y" + charMsgToWString(consoleBuffer) + L"\n";
	OutputDebugString(consoleOutput.c_str());*/

	if (ret < 0) {
		return;
	}
	if (ret >= sizeof bufferY) {
		consoleOutput = L"pos.y buffer too small\n";
	}

	char bufferZ[128];
	ret = snprintf(bufferZ, sizeof bufferZ, "%f ", z);
	/*consoleBuffer = string(bufferZ, 0, ret);
	consoleOutput = L"Pos.Z" + charMsgToWString(consoleBuffer) + L"\n";
	OutputDebugString(consoleOutput.c_str());*/

	if (ret < 0) {
		return;
	}
	if (ret >= sizeof bufferZ) {
		consoleOutput = L"pos.z buffer too small\n";
	}

	char buffer[4096];
	strncpy(buffer, bufferX, sizeof(bufferX));
	strncat(buffer, bufferY, sizeof(bufferY));
	strncat(buffer, bufferZ, sizeof(bufferZ));

	/*consoleBuffer = string(buffer, 0, sizeof buffer);
	consoleOutput = L"Pos" + charMsgToWString(consoleBuffer) + L"\n";
	OutputDebugString(consoleOutput.c_str());*/

	const char* msg = buffer;

	int sendResult = send(sock, msg, sizeof buffer, 0);

}