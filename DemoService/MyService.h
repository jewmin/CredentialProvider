#pragma once

#include "Header.h"
#include "IService.h"
#include "CriticalSection.h"

namespace DemoService {

class MyService : public Utils::IService {
public:
	typedef std::map<DWORD, Utils::SessionProperties *> SessionPropertiesDict;
	typedef std::map<DWORD, Utils::SessionProperties *>::iterator SessionPropertiesDictIter;
	MyService();
	~MyService();

	// ��ʼ���ص�
	virtual void OnInited();
	// ��Ȩ��֤
	virtual void Auth(Utils::Protocol::LoginRequest * request, Utils::Protocol::LoginResponse * response);
	// ��¼�ص�
	virtual void OnLogon(DWORD SessionID);
	// ע���ص�
	virtual void OnLogoff(DWORD SessionID);
	// �����ص�
	virtual void OnLock(DWORD SessionID);
	// �����ص�
	virtual void OnUnLock(DWORD SessionID);
	// �ػ�&�����ص�
	virtual void OnShutdown();

private:
	Utils::SessionProperties last_auth_user_;
	SessionPropertiesDict sessions_dict_;
	Utils::CCriticalSection cs_;
};

}
