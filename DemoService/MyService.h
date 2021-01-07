#pragma once

#include "Header.h"
#include "IService.h"
#include "CriticalSection.h"

namespace Utils {

class MyService : public IService {
public:
	MyService();
	~MyService();

	// ��ʼ���ص�
	virtual void OnInited();
	// ��¼�ص�
	virtual void OnLogon(SessionProperties session);
	// ע���ص�
	virtual void OnLogoff(SessionProperties session);
	// �����ص�
	virtual void OnLock(SessionProperties session);
	// �����ص�
	virtual void OnUnLock(SessionProperties session);
	// �ػ�&�����ص�
	virtual void OnShutdown();

private:
	std::map<DWORD, SessionProperties *> sessions_map_;
	CCriticalSection cs_;
};

}
