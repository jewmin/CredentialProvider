#pragma once

#include "UvHandler.h"
#include "Protocol/LoginRequest.h"
#include "Protocol/LoginResponse.h"
#include <credentialprovider.h>

// �ܵ��ͻ���
class AuthClient : public Utils::UvHandler {
public:
	AuthClient(DWORD timeout, IQueryContinueWithStatus* pqcws);
	virtual ~AuthClient();

	// ��Ȩ��֤
	void Auth(Utils::Protocol::LoginRequest * request, Utils::Protocol::LoginResponse * response);

protected:
	// Э�鴦��
	virtual void ProcessCommand(uv_pipe_t * pipe, const Utils::CIOBuffer * buffer);
	// ���ӳɹ��ص�
	virtual void OnConnected(uv_pipe_t * pipe, bool status);

private:
	DWORD timeout_;
	IQueryContinueWithStatus * pqcws_;
	Utils::Protocol::LoginRequest request_;
	Utils::Protocol::LoginResponse response_;
};
