#pragma once

#include "Header.h"

namespace Utils {

// ��ȡ�������
std::wstring GetMachineName();
// �ж��Ƿ����Ա�˺�
bool IsUserLocalAdmin(std::wstring username);
// ����ָ���Ự���û���
std::wstring GetSessionUserName(DWORD SessionId);
// ����ָ���Ự������
std::wstring GetSessionDomainName(DWORD SessionId);
// ��ȡ��ǰ�Ự
DWORD GetCurrentSessionId();

}
