#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<iostream>
#include<string>
#include<winsock2.h>

#include "../IOCPServer/protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

constexpr int CLIENT_NUM = 2;

void ShowErrorMessage(string message)
{
	cout << "[�����߻�]: " << message << '\n';

	cout << "ErrorCode: " << WSAGetLastError();

	system("pause");
	exit(1);
}

int main()
{
	SOCKET clientSocket[CLIENT_NUM];
	for (int i = 0; i < CLIENT_NUM; ++i) {
		WSADATA wsaData;

		char received[256];

		string sent;

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // Winsock���ʱ�ȭ�մϴ�.
			ShowErrorMessage("WSAStartup()");

		clientSocket[i] = socket(PF_INET, SOCK_STREAM, 0); // TCP �����������մϴ�.

		if (clientSocket[i] == INVALID_SOCKET)
			ShowErrorMessage("socket()");

		SOCKADDR_IN serverAddress;
		memset(&serverAddress, 0, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // ���ڿ�IP����Ʈ��ũ����Ʈ��������
		serverAddress.sin_port = htons(PORT_NUM); // 2����Ʈ������Ʈ��ũ����Ʈ��������

		if (connect(clientSocket[i], (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
			ShowErrorMessage("connect()");

		cout << "[Client]\n";

		CS_TEST_PACK test_pack;
		test_pack.size = sizeof(CS_TEST_PACK);
		test_pack.type = CS_TEST;

		string name = "TestName";
		strcpy_s(test_pack.BufferName, name.c_str());

		send(clientSocket[i], (char*)&test_pack, test_pack.size, NULL);
	}

	for (int i = 0; i < CLIENT_NUM; ++i) {
		CS_TEST_PACK test_pack;
		test_pack.size = sizeof(CS_TEST_PACK);
		test_pack.type = CS_TEST;

		string name = "Send NewTest Pack";
		strcpy_s(test_pack.BufferName, name.c_str());
		send(clientSocket[i], (char*)&test_pack, test_pack.size, NULL);
	}

	for (int i = 0; i < CLIENT_NUM; ++i) {
		closesocket(clientSocket[i]);
	}
	WSACleanup();
}