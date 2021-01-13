#include "MyService.h"
#include "Utils.h"

namespace DemoService {

MyService::MyService() {
	Utils::Output(L"MyService::MyService");
}

MyService::~MyService() {
	Utils::Output(L"MyService::~MyService");
	for (SessionPropertiesDictIter it = sessions_dict_.begin(); it != sessions_dict_.end(); it++) {
		delete it->second;
	}
}

void MyService::OnInited() {
	Utils::Output(L"��ʼ��");
}

void MyService::Auth(Utils::Protocol::LoginRequest * request, Utils::Protocol::LoginResponse * response) {
	Utils::CCriticalSection::Owner lock(cs_);

	wcscpy_s(response->UserName, request->UserName);
	wcscpy_s(response->Password, request->Password);
	static WORD testResult = Utils::Protocol::LoginResponse::Unknown;
	response->Result = testResult++ % 6;

	Utils::Output(Utils::StringFormat(L"�Ự[%u]��Ȩ�ɹ�", request->SessionID));
	if (last_auth_user_.UserName.empty()) {
		Utils::Output(Utils::StringFormat(L"���û�[%s]", request->UserName));
	} else if (last_auth_user_.UserName != request->UserName) {
		Utils::Output(Utils::StringFormat(L"�л��û�[%s] -> [%s]", last_auth_user_.UserName.c_str(), request->UserName));
	}
	last_auth_user_.SessionID = request->SessionID;
	last_auth_user_.UserName = request->UserName;
	last_auth_user_.Password = request->Password;
	last_auth_user_.LoginTime.Now();
}

void MyService::OnLogon(DWORD SessionID) {
	Utils::CCriticalSection::Owner lock(cs_);
	Utils::Output(Utils::StringFormat(L"�Ự[%u]��¼", SessionID));
	SessionPropertiesDictIter it = sessions_dict_.find(SessionID);
	if (it != sessions_dict_.end()) {
		Utils::Output(Utils::StringFormat(L"�����û���Ϣ[%s] -> [%s]", it->second->ToString().c_str(), last_auth_user_.ToString().c_str()));
		delete it->second;
		it->second = new Utils::SessionProperties(last_auth_user_);
	} else {
		sessions_dict_.insert({SessionID, new Utils::SessionProperties(last_auth_user_)});
	}
}

void MyService::OnLogoff(DWORD SessionID) {
	Utils::CCriticalSection::Owner lock(cs_);
	Utils::Output(Utils::StringFormat(L"�Ự[%u]ע��", SessionID));
	SessionPropertiesDictIter it = sessions_dict_.find(SessionID);
	if (it != sessions_dict_.end()) {
		Utils::Output(Utils::StringFormat(L"ɾ���û���Ϣ[%s]", it->second->ToString().c_str()));
		delete it->second;
		sessions_dict_.erase(it);
	}
}

void MyService::OnLock(DWORD SessionID) {
	Utils::CCriticalSection::Owner lock(cs_);
	Utils::Output(Utils::StringFormat(L"�Ự[%u]����", SessionID));
}

void MyService::OnUnLock(DWORD SessionID) {
	Utils::CCriticalSection::Owner lock(cs_);
	Utils::Output(Utils::StringFormat(L"�Ự[%u]����", SessionID));
	SessionPropertiesDictIter it = sessions_dict_.find(SessionID);
	if (it != sessions_dict_.end()) {
		if (!it->second->LoginTime.IsInited()) {
			Utils::Output(Utils::StringFormat(L"�����û���Ϣ[%s] -> [%s]", it->second->ToString().c_str(), last_auth_user_.ToString().c_str()));
			delete it->second;
			it->second = new Utils::SessionProperties(last_auth_user_);
		}
	}
}

void MyService::OnShutdown() {
	Utils::Output(L"�ػ�");
}

}
