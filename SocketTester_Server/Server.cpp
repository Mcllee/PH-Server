#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    SOCKADDR_IN serverAddr, clientAddr;
    char buffer[1024];

    // Winsock �ʱ�ȭ
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "Failed to initialize winsock." << std::endl;
        return -1;
    }

    // ���� ����
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cout << "Failed to create socket." << std::endl;
        return -1;
    }

    // ���� �ּ� ����
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // ���� ���ε�
    if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "Failed to bind socket." << std::endl;
        closesocket(serverSocket);
        return -1;
    }

    // Ŭ���̾�Ʈ ���� ���
    listen(serverSocket, 1);
    std::cout << "Server listening on port 8888..." << std::endl;

    // Ŭ���̾�Ʈ ���� ����
    int clientAddrSize = sizeof(clientAddr);
    if ((clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &clientAddrSize)) == INVALID_SOCKET) {
        std::cout << "Failed to accept client connection." << std::endl;
        closesocket(serverSocket);
        return -1;
    }
    std::cout << "Client connected." << std::endl;

    // Ŭ���̾�Ʈ�κ��� ������ ���� �� �۽�
    while (true) {
        memset(buffer, 0, sizeof(buffer));

        // Ŭ���̾�Ʈ�κ��� ������ ����
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead == SOCKET_ERROR) {
            std::cout << "Failed to receive data from client." << std::endl;
            break;
        }

        // Ŭ���̾�Ʈ�κ��� ���� ������ ���
        std::cout << "Received from client: " << buffer << std::endl;

        // Ŭ���̾�Ʈ�� ������ �۽�
        if (send(clientSocket, buffer, bytesRead, 0) == SOCKET_ERROR) {
            std::cout << "Failed to send data to client." << std::endl;
            break;
        }
    }

    // ���� ���� �� ���� ����
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
