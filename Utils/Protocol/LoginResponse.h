#pragma once

#include "Commond.h"

namespace Utils {

namespace Protocol {

// ��Ȩ������ӦЭ��
struct LoginResponse {
	static const WORD Unknown = 0;
	static const WORD AuthFailed = 1;
	static const WORD AuthSuccess = 2;
	static const WORD UserCancel = 3;
	static const WORD Timeout = 4;
	static const WORD ConnectFailed = 5;

	// ��Ȩ���
	WORD Result;
	// ����ϵͳ�û���
	wchar_t UserName[MAX_STRING_SIZE];
	// ����ϵͳ����
	wchar_t Password[MAX_STRING_SIZE];
	// ������������к�
	int Sn;

	LoginResponse()
		: Result(Unknown), Sn(0) {
		wmemset(UserName, 0, sizeof(UserName) / sizeof(wchar_t));
		wmemset(Password, 0, sizeof(Password) / sizeof(wchar_t));
	}
};

}

}
