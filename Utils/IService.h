#pragma once

namespace Utils {

class IService {
public:
	// ��ʼ���ص�
	virtual void OnInited() = 0;
	// ��¼�ص�
	virtual void OnLogon() = 0;
	// ע���ص�
	virtual void OnLogoff() = 0;
	// �����ص�
	virtual void OnLock() = 0;
	// �����ص�
	virtual void OnUnLock() = 0;
	// �ػ�&�����ص�
	virtual void OnShutdown() = 0;
};

}
