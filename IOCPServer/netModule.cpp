#pragma once

#include "DBModule.h"

std::array<SESSION, MAX_USER> clients;	// �÷��̾�'s �����̳�
std::array<PARTY, MAX_PARTY> partys;

void process_packet(short c_uid, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN:
	{
		CS_LOGIN_PACK* p = reinterpret_cast<CS_LOGIN_PACK*>(packet);
		short new_c_uid = -1;
		char db_name[CHAR_SIZE], db_skill[4];
		char db_skin, db_pet, db_item;

		// AWS Cognito ���� ������ ����ڸ� ����
		if (Login_UDB(p->id, p->pw, new_c_uid, db_name, db_skin, db_pet, db_item, db_skill) && new_c_uid != -1) {
			clients[c_uid].set_name(db_name);
			
			{
				std::lock_guard<std::mutex> ll{ clients[c_uid]._lock };
				clients[c_uid]._state = ST_INGAME;
			}

			SC_LOGIN_SUCCESS_PACK ok_pack;
			ok_pack.size = sizeof(ok_pack);
			ok_pack.type = SC_LOGIN_SUCCESS;

			for (SESSION& cl : clients) {
				{
					std::lock_guard<std::mutex> ll{ cl._lock };
					if (ST_INGAME != cl._state) continue;
					// Ŭ���̾�Ʈ�� ���°� INGAME�� �ƴ϶�� ��Ŷ�� ���� �ʿ䰡 ����
				}

				if (0 == strcmp(cl.get_name(), clients[c_uid].get_name())) {
					cl.do_send(&ok_pack);
					std::cout << "Send Ok Packet!\n";
				}
			}

			SC_LOGIN_INFO_PACK info_pack;
			info_pack.size = sizeof(SC_LOGIN_INFO_PACK);
			info_pack.type = SC_LOGIN_INFO;
			strncpy_s(info_pack.name, sizeof(info_pack.name), clients[c_uid].get_name() + '\0', sizeof(clients[c_uid].get_name()) + 1);
			info_pack._player_skin = db_skin;
			info_pack._pet_num = db_pet;
			info_pack._q_item = db_item;
			strncpy_s(info_pack._q_skill, sizeof(info_pack._q_skill), db_skill + '\0', sizeof(db_skill) + 1);
		
			// ���� ������ Ŭ���̾�Ʈ ������ �ٸ� ���� Ŭ���̾�Ʈ�鿡�� ����
			for (SESSION& cl : clients) {
				if (0 != strcmp(cl.get_name(), "empty")) {
					// �۽� ���ʿ� ���
					if (ST_INGAME != cl._state)	continue;
					if (c_uid == cl._uid)		continue;
					// �۽� �ʿ� ���
					cl.do_send(&info_pack);
				}
			}

			SC_LOGIN_INFO_PACK old_info_pack;
			old_info_pack.size = sizeof(SC_LOGIN_INFO_PACK);
			old_info_pack.type = SC_LOGIN_INFO;

			// ���ο� Ŭ���̾�Ʈ���� ���� Ŭ���̾�Ʈ�� ������ ���� ����
			for (SESSION& cl : clients) {
				// ���ο� Ŭ���̾�Ʈ���� �ڱ� �ڽ��� ������ ���� �ʿ� ����(�� �ݺ����� �ߺ���)
				if (new_c_uid != cl._uid) { 
					old_info_pack.name;
					// strncpy_s(old_info_pack.name, c._name, CHAR_SIZE);

					if (ST_INGAME != cl._state)	continue;
					if (c_uid == cl._uid)				continue;

					// ���ο� Ŭ���̾�Ʈ���� ����
					clients[new_c_uid].do_send(&old_info_pack);
				}
			}
			std::cout << "Login Success!\n";
		}
		else {
			SC_LOGIN_FAIL_PACK fail_pack;
			fail_pack.size = sizeof(SC_LOGIN_FAIL_PACK);
			fail_pack.type = SC_LOGIN_FAIL;
			clients[new_c_uid].do_send(&fail_pack);

			std::cout << "Login Fail!\n";
		}
 	}
	break;
	case CS_QUEST_INVENTORY:
	{
		// DB���� c_uid�� �ش��ϴ� ������ ���� ��������
		Get_IDB(c_uid);

		// ������ ������ ��Ŷ
		// �����, Zeromemory�� �ʱ�ȭ �ʿ����� Ȯ�� �ʿ�.(������ ���� ����)
		SC_ITEM_INFO_PACK item_pack;
		item_pack.size = sizeof(SC_ITEM_INFO_PACK);
		item_pack.type = SC_ITEM_INFO;

		for (short i = 0; i < MAX_ITEM_COUNT; ++i) {
			if (0 < clients[c_uid].Collection[i]) {
				strncpy_s(item_pack._name, CHAR_SIZE, (Get_ItemID(1, false) + std::format("{0:0>2}", i)).c_str(), 4);
				item_pack._cnt = clients[c_uid].Collection[i];
			}
			if (0 < clients[c_uid].Install[i]) {
				strncpy_s(item_pack._name, CHAR_SIZE, (Get_ItemID(2, false) + std::format("{0:0>2}", i)).c_str(), 4);
				item_pack._cnt = clients[c_uid].Install[i];
			}
			if (0 < clients[c_uid].Launcher[i]) {
				strncpy_s(item_pack._name, CHAR_SIZE, (Get_ItemID(3, false) + std::format("{0:0>2}", i)).c_str(), 4);
				item_pack._cnt = clients[c_uid].Launcher[i];
			}
			if (0 < clients[c_uid].Potion[i]) {
				strncpy_s(item_pack._name, CHAR_SIZE, (Get_ItemID(4, false) + std::format("{0:0>2}", i)).c_str(), 4);
				item_pack._cnt = clients[c_uid].Potion[i];
			}
			clients[c_uid].do_send(&item_pack);
		}
		// name�� theEnd�� ������ ������ ������ ������� �˸�(��Ŷ�� ����)
		strncpy_s(item_pack._name, CHAR_SIZE, "theEnd", sizeof("theEnd"));
		clients[c_uid].do_send(&item_pack);
	}
	break;
	case CS_PARTY_SEARCHING:
	{
		//send party list
		SC_PARTIES_INFO_PACK party_list;
		party_list.size = sizeof(SC_PARTIES_INFO_PACK);
		party_list.type = SC_PARTY_LIST_INFO;

		strcpy_s(party_list._name, "TEST PARTY");
		party_list._staff_count = 1;	// char�� ����

		clients[c_uid].do_send(&party_list);
	}
	break;
	case CS_PARTY_INFO:	// �÷��̾ ��Ƽ�뿡 ����
	{
		CS_PARTY_INFO_PACK* party_info = reinterpret_cast<CS_PARTY_INFO_PACK*>(packet);

		short party_num = static_cast<int>(party_info->party_num);
		
		std::array<char[CHAR_SIZE], 4> member;
		std::array<char, 4> pets;
		partys[party_num].get_party_info(member, pets);

		// get partys[party_num] info
		SC_PARTY_INFO_PACK in_party;
		in_party.size = sizeof(SC_PARTY_INFO_PACK);
		in_party.type = SC_PARTY_INFO;

		if (0 <= party_num) {
			short staff_member = partys[party_num].get_member_count();
			for (int i = 0; i < staff_member; ++i) {
				strncpy_s(in_party._mem[i], sizeof(in_party._mem[i]), member[i], sizeof(member[i]));
				in_party._mem_pet[i] = pets[i];
			}
		}

		clients[c_uid].do_send(&in_party);
	}
	break;
	case CS_PARTY_READY:	// ��Ƽ ���� �غ�Ϸ�
	{

	}
	break;
	case CS_PARTY_JOIN:		// ��Ƽ�� Survival Stage�� �����Ѵٴ� ��ȣ
	{
		CS_PARTY_JOIN_PACK* new_staff = reinterpret_cast<CS_PARTY_JOIN_PACK*>(packet);

		char* staff_name{};
		char staff_pet = static_cast<char>(1);
		strncpy_s(staff_name, sizeof(staff_name), "empty", sizeof("empty"));

		// partys[new_staff->party_num].new_member();
	}
	break;
	case CS_PARTY_LEAVE:
	{
		CS_PARTY_LEAVE_PACK* old_staff = reinterpret_cast<CS_PARTY_LEAVE_PACK*>(packet);

		if (/*checking data && save data*/ partys[old_staff->party_num].leave_member(old_staff->name)) {
			SC_PARTY_LEAVE_SUCCESS_PACK leave_pack;
			leave_pack.size = sizeof(SC_PARTY_LEAVE_SUCCESS_PACK);
			leave_pack.type = SC_PARTY_LEAVE_SUCCESS;
			clients[c_uid].do_send(&leave_pack);
		}
		else {
			SC_PARTY_LEAVE_FAIL_PACK fail_leave_pack;
			fail_leave_pack.size = sizeof(SC_PARTY_LEAVE_FAIL_PACK);
			fail_leave_pack.type = SC_PARTY_LEAVE_FAIL;
			clients[c_uid].do_send(&fail_leave_pack);
		}
	}
	break;
	case CS_LOGOUT:
	{
		// Disconnect client
		CS_LOGOUT_PACK* logout_client = reinterpret_cast<CS_LOGOUT_PACK*>(packet);

		if (/*checking_DB(logout_client->name, c_uid)*/false) {
			SC_LOGOUT_SUCCESS_PACK out_client;
			out_client.size = sizeof(SC_LOGOUT_SUCCESS_PACK);
			out_client.type = SC_LOGOUT_SUCCESS;
			clients[c_uid].do_send(&out_client);
		}
		else {
			SC_LOGOUT_FAIL_PACK return_client;
			return_client.size = sizeof(SC_LOGOUT_FAIL_PACK);
			return_client.type = SC_LOGOUT_FAIL;
			clients[c_uid].do_send(&return_client);
		}
	}
	break;
	}
}

void disconnect(short c_uid)
{
	for (SESSION& cl : clients) {
		{
			std::lock_guard<std::mutex> ll(cl._lock);
			if (ST_INGAME == cl._state) continue;
		}
		if (c_uid == cl._uid) {
			// logout logic
			continue;
		}

		// send logout packet
	}

	// client ���� ����
	closesocket(clients[c_uid]._socket);
	std::lock_guard<std::mutex> ll(clients[c_uid]._lock);
	clients[c_uid]._state = ST_FREE;
}

void worker_thread(HANDLE h_iocp)
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);

		// error �����
		if (FALSE == ret) {
			if (ex_over->c_type == ACCEPT) {
				std::cout << "Accept error\n";
			}
			else {
				std::cout << "GQCS Error on client[" << key << "]\t";
				// disconnect client
			}
		}
		// if ((0 == num_bytes) && (ex_over->c_type == RECV)) continue;

		switch (ex_over->c_type) {
		case ACCEPT:	// accept new client
		{
			// newClient ������ȣ �ο�, �������� �Է�, ����
			short new_c_uid = get_player_uid();

			if (-1 != new_c_uid) { // ���� ����, ���� �ޱ�
				{
					std::lock_guard<std::mutex> ll{ clients[new_c_uid]._lock };
					clients[new_c_uid]._state = ST_ALLOC;
				}

				clients[new_c_uid]._socket = g_c_socket;

				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket), h_iocp, NULL, 0);
				clients[new_c_uid].do_recv();
				g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

				ZeroMemory(&g_a_over._over, sizeof(g_a_over._over));
				int addr_size = sizeof(SOCKADDR_IN);
				AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);

				// Dummy Client Ȯ�ο�
				std::cout << "NEW PLAYER!\n";
			}
			else {					// ���� ����
				std::cout << "connect fail\n";
				break;
			}
		}
		break;
		case RECV:		// get new message
		{
			// ��Ŷ ������
			int remain_data = num_bytes + clients[key]._prev_size;
			char* p = ex_over->_send_buf;

			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					process_packet(static_cast<int>(key), p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}

			clients[key]._prev_size = remain_data;

			if (remain_data > 0) {
				memcpy(ex_over->_send_buf, p, remain_data);
			}
			clients[key].do_recv();
		}
		break;
		case SEND:		// send new message
		{
			std::cout << "SEND\n";
			delete ex_over;
		}
		break;
		}
	}
}