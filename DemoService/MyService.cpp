#include "MyService.h"
#include "Utils.h"

namespace Utils {

MyService::MyService() {
	Utils::Output(L"MyService::MyService");
}

MyService::~MyService() {
	Utils::Output(L"MyService::~MyService");
	for (std::map<DWORD, SessionProperties *>::iterator it = sessions_map_.begin(); it != sessions_map_.end(); it++) {
		delete it->second;
	}
}

void MyService::OnInited() {
	Utils::Output(L"��ʼ��");
}

void MyService::OnLogon(SessionProperties session) {
	Utils::CCriticalSection::Owner lock(cs_);
	Utils::Output(Utils::StringFormat(L"�Ự[%s(%u)]��¼", session.UserName.c_str(), session.SessionID));
	std::map<DWORD, SessionProperties *>::iterator it = sessions_map_.find(session.SessionID);
	if (it != sessions_map_.end()) {
		Utils::Output(Utils::StringFormat(L"�л��û�[%s] -> [%s]", it->second->UserName.c_str(), session.UserName.c_str()));
		delete it->second;
		it->second = new SessionProperties(session);
	} else {
		sessions_map_.insert({session.SessionID, new SessionProperties(session)});
	}
}

void MyService::OnLogoff(SessionProperties session) {
	Utils::CCriticalSection::Owner lock(cs_);
	std::map<DWORD, SessionProperties *>::iterator it = sessions_map_.find(session.SessionID);
	if (it != sessions_map_.end()) {
		Utils::Output(Utils::StringFormat(L"�Ự[%s(%u)]ע��", it->second->UserName.c_str(), it->first));
		delete it->second;
		sessions_map_.erase(it);
	} else {
		Utils::Output(Utils::StringFormat(L"�Ự[%u]ע��", session.SessionID));
	}
}

void MyService::OnLock(SessionProperties session) {
	Utils::CCriticalSection::Owner lock(cs_);
	std::map<DWORD, SessionProperties *>::iterator it = sessions_map_.find(session.SessionID);
	if (it != sessions_map_.end()) {
		Utils::Output(Utils::StringFormat(L"�Ự[%s(%u)]����", it->second->UserName.c_str(), it->first));
	} else {
		Utils::Output(Utils::StringFormat(L"�Ự[%u]����", session.SessionID));
	}
}

void MyService::OnUnLock(SessionProperties session) {
	Utils::CCriticalSection::Owner lock(cs_);
	std::map<DWORD, SessionProperties *>::iterator it = sessions_map_.find(session.SessionID);
	if (it != sessions_map_.end()) {
		Utils::Output(Utils::StringFormat(L"�Ự[%s(%u)]����", it->second->UserName.c_str(), it->first));
	} else {
		Utils::Output(Utils::StringFormat(L"�Ự[%u]����", session.SessionID));
	}
}

void MyService::OnShutdown() {
	Utils::Output(L"�ػ�");
}

}
