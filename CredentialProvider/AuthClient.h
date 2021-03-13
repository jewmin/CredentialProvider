#pragma once

#include "UvHandler.h"
#include "Thread.h"
#include "Protocol/LoginRequest.h"
#include "Protocol/LoginResponse.h"
#include <credentialprovider.h>
#include "CriticalSection.h"

// �ܵ��ͻ���
class CSampleProvider;
class AuthClient : public Utils::UvHandler, Utils::CThread {
public:
	AuthClient(int timeout);
	virtual ~AuthClient();

	// ��ʼ��
	HRESULT Initialize(CSampleProvider * pProvider);
	// ��Ȩ��֤
	void Auth(Utils::Protocol::LoginRequest * request);

protected:
	// Э�鴦��
	virtual void ProcessCommand(uv_pipe_t * pipe, const Utils::CIOBuffer * buffer);
	// ���ӳɹ��ص�
	virtual void OnConnected(uv_pipe_t * pipe, bool status);
	// �Ͽ����ӻص�
	virtual void OnDisconnected(uv_pipe_t * pipe);
	// �̺߳���
	virtual void OnRountine();
	// ������Ȩ����
	void SendAuth();

private:
	static void TimerCb(uv_timer_t* handle);
	static void AsyncCb(uv_async_t* handle);

private:
	int timeout_;
	uv_pipe_t * pipe_;
	uv_timer_t reconnect_timer_;
	uv_async_t queue_notify_;
	CSampleProvider * pProvider_;
	Utils::Protocol::LoginRequest last_request_;
	Utils::CCriticalSection cs_;
	std::vector<Utils::Protocol::LoginRequest> request_queue_;
};
