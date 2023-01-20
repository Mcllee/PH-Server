#pragma once
#include "stdafx.h"
#include "netModule.h"

int main() {
	WSADATA WSAData;

	int err_code;
	err_code = WSAStartup(MAKEWORD(2, 2), &WSAData);

	/*
	Overlapped I/O -> IOCP 

	IOCP = non-blocking + asynchronous
	SELECT = blocking + asynchronous

	[Overlapped I/O]
	Overlapped I/O �� �����͸� ���ÿ� �ް�
	Completion Routine �Լ��� �����͸� ó���Ѵ�.

	[IOCP]
	Input/Output Completion Port�� ����
	Port�� ��Ʈ��ũ ���� �ǹ̰� �ƴ� �۾�, ���� ���� ó���ϴ� ��ü
	�Է��� �Ϸ�Ǹ�(Overlapped I/O ������ ��� ������) Completion Queue�� ���δ�.
	Completion Queue�� �۾��� ���̸�(�����) Worker Thread �� ���� ������ ó��

	IOCP�� Overlapped I/O ���� Ȯ�����̱� ������ Overlapped I/O�� ����ü�� �Լ��� ���

	[�ʼ� �Լ�]
	1. CreateIoCompletionPort
	2. GetQueuedCompletionPort
	3. PostQueuedCompletionPort
	*/
	


	WSACleanup();
}
