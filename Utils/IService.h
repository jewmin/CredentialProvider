#pragma once

#include "SessionProperties.h"
#include "Protocol/LoginRequest.h"
#include "Protocol/LoginResponse.h"

namespace Utils {

class IService {
public:
	// ��ʼ���ص�
	virtual void OnInited() = 0;
	// ��Ȩ��֤
	virtual void Auth(Utils::Protocol::LoginRequest * request, Utils::Protocol::LoginResponse * response) = 0;
	// ��¼�ص�
	virtual void OnLogon(DWORD SessionID) = 0;
	// ע���ص�
	virtual void OnLogoff(DWORD SessionID) = 0;
	// �����ص�
	virtual void OnLock(DWORD SessionID) = 0;
	// �����ص�
	virtual void OnUnLock(DWORD SessionID) = 0;
	// �ػ�&�����ص�
	virtual void OnShutdown() = 0;
};

}
