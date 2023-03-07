#pragma once

#include "netModule.h"
#include "DBModule.h"

void process_packet(short c_uid, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN:
	{
		CS_LOGIN_PACK* p = reinterpret_cast<CS_LOGIN_PACK*>(packet);
		short new_c_uid = -1;

		// DB data checking
		if (/*DB*/true && new_c_uid != -1) {
			// Login Sucssess
			
			SC_LOGIN_SUCCESS_PACK ok_pack;
			ok_pack.size = sizeof(ok_pack);
			ok_pack.type = SC_LOGIN_SUCCESS;
			clients[new_c_uid].do_send(&ok_pack);

			SC_LOGIN_INFO_PACK info_pack;
			info_pack.size = sizeof(SC_LOGIN_INFO_PACK);
			info_pack.type = SC_LOGIN_INFO;

			// ���� ������ Ŭ���̾�Ʈ ������ �ٸ� ���� Ŭ���̾�Ʈ�鿡�� ����
			for (SESSION& c : clients) {
				c.do_send(&info_pack);
			}

			SC_LOGIN_INFO_PACK old_info_pack;
			old_info_pack.size = sizeof(SC_LOGIN_INFO_PACK);
			old_info_pack.type = SC_LOGIN_INFO;

			// ���ο� Ŭ���̾�Ʈ���� ���� Ŭ���̾�Ʈ�� ������ ���� ����
			for (SESSION& c : clients) {
				// ���ο� Ŭ���̾�Ʈ���� �ڱ� �ڽ��� ������ ���� �ʿ� ����(�� �ݺ����� �ߺ���)
				if (new_c_uid != c._uid) {
					old_info_pack.name;
					old_info_pack.x = c._x;
					old_info_pack.y = c._y;
					old_info_pack.z = c._z;
					strncpy_s(old_info_pack.name, c._name, CHAR_SIZE);

					// ���ο� Ŭ���̾�Ʈ���� ����
					clients[new_c_uid].do_send(&old_info_pack);
				}
			}
		}
		else if(new_c_uid != -1){
			SC_LOGIN_FAIL_PACK fail_pack;
			fail_pack.size = sizeof(SC_LOGIN_FAIL_PACK);
			fail_pack.type = SC_LOGIN_FAIL;
			clients[new_c_uid].do_send(&fail_pack);
		}
	}
	break;
	case CS_SEARCHING_PARTY:
	{
		//send party list
		SC_PARTY_LIST_INFO_PACK party_list;
		party_list.size = sizeof(SC_PARTY_LIST_INFO_PACK);
		party_list.type = SC_PARTY_LIST_INFO;
		strcpy_s(party_list._name, "TEST PARTY");
		party_list._staff_member = 1;	// char�� ����

		clients[c_uid].do_send(&party_list);
	}
	break;
	case CS_PARTY_INFO:	// �÷��̾ ��Ƽ�뿡 ����
	{
		CS_PARTY_INFO_PACK* party_info = reinterpret_cast<CS_PARTY_INFO_PACK*>(packet);

		short party_num = static_cast<int>(party_info->party_num);
		
		array<char[CHAR_SIZE], 4> member;
		array<char, 4> pets;
		partys[party_num].get_party_info(member, pets);

		// get partys[party_num] info
		SC_PARTY_INFO_PACK in_party;
		in_party.size = sizeof(SC_PARTY_INFO_PACK);
		in_party.type = SC_PARTY_INFO;

		if (0 <= party_num) {
			short staff_member = partys[party_num].get_member_count();
			for (int i = 0; i < staff_member; ++i) {
				strcpy(in_party._mem[i], member[i]);
				in_party._mem_pet[i] = pets[i];
			}
		}

		clients[c_uid].do_send(&in_party);
	}
	break;
	case CS_JOIN_PARTY:
	{
		CS_JOIN_PARTY_PACK* new_staff = reinterpret_cast<CS_JOIN_PARTY_PACK*>(packet);

		char* staff_name{};
		char staff_pet = static_cast<char>(1);
		strcpy(staff_name, "empty");

		partys[new_staff->party_num].new_member(staff_name, staff_pet);
	}
	break;
	case CS_LEAVE_PARTY:
	{
		CS_LEAVE_PARTY_PACK* old_staff = reinterpret_cast<CS_LEAVE_PARTY_PACK*>(packet);

		checking_DB(old_staff->name, c_uid);
		
		if (/*checking data && save data*/ partys[old_staff->party_num].out_party_staff(old_staff->name)) {
			SC_LEAVE_PARTY_SUCCESS_PACK leave_pack;
			leave_pack.size = sizeof(SC_LEAVE_PARTY_SUCCESS_PACK);
			leave_pack.type = SC_LEAVE_PARTY_SUCCESS;
			clients[c_uid].do_send(&leave_pack);
		}
		else {
			SC_LEAVE_PARTY_FAIL_PACK fail_leave_pack;
			fail_leave_pack.size = sizeof(SC_LEAVE_PARTY_FAIL_PACK);
			fail_leave_pack.type = SC_LEAVE_PARTY_FAIL;
			clients[c_uid].do_send(&fail_leave_pack);
		}
	}
	break;
	case CS_LOGOUT:
	{
		// Disconnect client
		CS_LOGOUT_PACK* logout_client = reinterpret_cast<CS_LOGOUT_PACK*>(packet);

		if (checking_DB(logout_client->name, c_uid)) {
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
				std::string err_text = "errorCollecter.txt";
				std::ofstream err_file(err_text.data());

				if (err_file.is_open()) {
					err_file << "";

					time_t timer;
					struct tm* t;

					timer = time(NULL);
					t = localtime(&timer);

					err_file << t->tm_year + 1900 << "/" << t->tm_mon + 1 << "/" << t->tm_mday
						<< "-(" << t->tm_hour << ":" << t->tm_min << ":" << t->tm_sec << ")"
						<< " : Accept Error\n";
				}
			}
			else continue;
		}
		if ((0 == num_bytes) && (ex_over->c_type == RECV)) continue;

		switch (ex_over->c_type) {
		case ACCEPT:	// accept new client
		{
			// newClient ������ȣ �ο�, �������� �Է�, ����
			short new_c_uid = get_player_uid();
			
			if (-1 != new_c_uid) { // ���� ����, ���� �ޱ�
				clients[new_c_uid]._x;
				clients[new_c_uid]._y;
				clients[new_c_uid]._z;
				clients[new_c_uid].direction;

				clients[new_c_uid]._uid = new_c_uid;
				clients[new_c_uid]._socket = g_c_socket;

				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket), h_iocp, new_c_uid, 0);
				clients[new_c_uid].do_recv();
				g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

			}
			else {					// ���� ����

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

		}
		break;
		}
	}
}