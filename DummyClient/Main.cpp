#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")

using namespace std;

int main() {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    // Winsock �ʱ�ȭ
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        // �ʱ�ȭ ����
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // ���� �ּ� ���� ��������
    iResult = getaddrinfo("localhost", "27015", &hints, &result);
    if (iResult != 0) {
        // �ּ� ���� �������� ����
    }

    // ������ ����
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // ���� ����
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            // ���� ���� ����
            continue;
        }

        // ������ ����
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            // ���� ����
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    // �ּ� ���� �޸� ����
    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        // ���� ���� ����
    }
}