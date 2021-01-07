#pragma once

#include "SessionProperties.h"

namespace Utils {

class IService {
public:
	// ��ʼ���ص�
	virtual void OnInited() = 0;
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
