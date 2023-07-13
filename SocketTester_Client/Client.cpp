#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    SOCKADDR_IN serverAddr;
    char buffer[1024];

    // Winsock �ʱ�ȭ
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "Failed to initialize winsock." << std::endl;
        return -1;
    }

    // ���� ����
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cout << "Failed to create socket." << std::endl;
        return -1;
    }

    // ���� �ּ� ����
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    serverAddr.sin_addr.s_addr = inet_addr("172.31.176.1"); // ���� IP �ּ�

    // ������ ����
    if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "Failed to connect to server." << std::endl;
        closesocket(clientSocket);
        return -1;
    }
    std::cout << "Connected to server." << std::endl;

    // �����κ��� ������ �۽� �� ����
    while (true) {
        std::cout << "Enter message: ";
        std::cin.getline(buffer, sizeof(buffer));

        // ������ ������ �۽�
        if (send(clientSocket, buffer, strlen(buffer), 0) == SOCKET_ERROR) {
            std::cout << "Failed to send data to server." << std::endl;
            break;
        }

        // �����κ��� ������ ����
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead == SOCKET_ERROR) {
            std::cout << "Failed to receive data from server." << std::endl;
            break;
        }

        // �����κ��� ���� ������ ���
        std::cout << "Received from server: " << buffer << std::endl;
    }

    // ���� ���� �� ���� ����
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
