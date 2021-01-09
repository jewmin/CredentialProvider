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
	virtual Protocol::LoginResponse Auth(Protocol::LoginRequest request) = 0;
	// ��¼�ص�
	virtual void OnLogon(SessionProperties session) = 0;
	// ע���ص�
	virtual void OnLogoff(SessionProperties session) = 0;
	// �����ص�
	virtual void OnLock(SessionProperties session) = 0;
	// �����ص�
	virtual void OnUnLock(SessionProperties session) = 0;
	// �ػ�&�����ص�
	virtual void OnShutdown() = 0;
};

}
