#include "ServiceBase.h"
#include "Utils.h"

namespace Utils {

CServiceBase * CServiceBase::ServiceInstance = NULL;

bool CServiceBase::Run(CServiceBase * service) {
	if (!service) {
		return false;
	}

	ServiceInstance = service;
	SERVICE_TABLE_ENTRY ServiceStartTable[] = {
		{ const_cast<LPWSTR>(ServiceInstance->service_name_.c_str()), CServiceBase::ServiceMain },
		{ NULL, NULL }
	};

	if (!::StartServiceCtrlDispatcher(ServiceStartTable)) {
		return false;
	}

	return true;
}

VOID WINAPI CServiceBase::ServiceMain(DWORD dwNumServicesArgs, LPWSTR * lpServiceArgVectors) {
	assert(NULL != ServiceInstance);
	ServiceInstance->service_status_handle_ = ::RegisterServiceCtrlHandlerEx(ServiceInstance->service_name_.c_str(), CServiceBase::ServiceCtrlHandler, reinterpret_cast<LPVOID>(ServiceInstance));
	if (ServiceInstance->service_status_handle_) {
		ServiceInstance->Run(dwNumServicesArgs, lpServiceArgVectors);
	}
	delete ServiceInstance;
	ServiceInstance = NULL;
}

DWORD WINAPI CServiceBase::ServiceCtrlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext) {
	if (!lpContext) {
		return ERROR_INVALID_PARAMETER;
	}

	CServiceBase * service = reinterpret_cast<CServiceBase *>(lpContext);
	if (!service) {
		return ERROR_INVALID_PARAMETER;
	}

	DWORD dwRet = NO_ERROR;
	switch (dwControl) {
	case SERVICE_CONTROL_STOP:
		service->ReportServiceStatus(SERVICE_STOP_PENDING);
		service->OnStop();
		break;

	case SERVICE_CONTROL_PAUSE:
		service->ReportServiceStatus(SERVICE_PAUSE_PENDING);
		service->OnPause();
		service->ReportServiceStatus(SERVICE_PAUSED);
		break;

	case SERVICE_CONTROL_CONTINUE:
		service->ReportServiceStatus(SERVICE_CONTINUE_PENDING);
		service->OnContinue();
		service->ReportServiceStatus(SERVICE_RUNNING);
		break;

	case SERVICE_CONTROL_INTERROGATE:
		service->ReportServiceStatus(service->service_status_.dwCurrentState, service->service_status_.dwWin32ExitCode);
		break;

	case SERVICE_CONTROL_SHUTDOWN:
		service->OnShutdown();
		service->ReportServiceStatus(service->service_status_.dwCurrentState, service->service_status_.dwWin32ExitCode);
		break;

	case SERVICE_CONTROL_POWEREVENT:
		if (dwEventType == PBT_POWERSETTINGCHANGE) {
			service->OnPowerEvent(PBT_POWERSETTINGCHANGE, reinterpret_cast<PPOWERBROADCAST_SETTING>(lpEventData));
		} else {
			service->OnPowerEvent(dwEventType, NULL);
		}
		service->ReportServiceStatus(service->service_status_.dwCurrentState, service->service_status_.dwWin32ExitCode);
		break;

	case SERVICE_CONTROL_SESSIONCHANGE:
		service->OnSessionChange(dwEventType, reinterpret_cast<PWTSSESSION_NOTIFICATION>(lpEventData));
		service->ReportServiceStatus(service->service_status_.dwCurrentState, service->service_status_.dwWin32ExitCode);
		break;

	default:
		if (dwControl >= 128 && dwControl <= 255) {
			service->OnCustomCommand(dwControl);
			service->ReportServiceStatus(service->service_status_.dwCurrentState, service->service_status_.dwWin32ExitCode);
		} else {
			dwRet = ERROR_CALL_NOT_IMPLEMENTED;
		}
		break;
	}

	return dwRet;
}

CServiceBase::CServiceBase(const std::wstring & service_name)
	: service_name_(service_name)
	, can_stop_(true)
	, can_shutdown_(false)
	, can_pause_and_continue_(false)
	, can_handle_power_event_(false)
	, can_handle_session_change_event_(false)
	, controls_accepted_(0) {
	log_.SetLogFileNameFormat(L"Service_%02d%02d%02d.log");
	SetLog(&log_);
	service_status_.dwServiceType = SERVICE_WIN32;
	service_status_.dwServiceSpecificExitCode = NO_ERROR;
	Utils::Output(Utils::StringFormat(L"CServiceBase::CServiceBase %s", service_name_.c_str()));
}

CServiceBase::~CServiceBase() {
	Utils::Output(L"CServiceBase::~CServiceBase");
	SetLog(NULL);
}

void CServiceBase::Run(DWORD dwNumServicesArgs, LPWSTR * lpServiceArgVectors) {
	Utils::Output(Utils::StringFormat(L"CServiceBase::Run dwNumServicesArgs: %u", dwNumServicesArgs));
	controls_accepted_ = UpdateControlsAccepted();
	ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, 30000);
	OnStart(dwNumServicesArgs, lpServiceArgVectors);
	ReportServiceStatus(SERVICE_RUNNING);
	RunService();
	ReportServiceStatus(SERVICE_STOPPED);
}

void CServiceBase::ReportServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint) {
	Utils::Output(Utils::StringFormat(L"CServiceBase::ReportServiceStatus dwCurrentState: %u, dwWin32ExitCode: %u, dwWaitHint: %u", dwCurrentState, dwWin32ExitCode, dwWaitHint));
	service_status_.dwCurrentState = dwCurrentState;
	service_status_.dwWin32ExitCode = dwWin32ExitCode;
	service_status_.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING) {
		service_status_.dwControlsAccepted = 0;
	} else {
		service_status_.dwControlsAccepted = controls_accepted_;
	}

	if (dwCurrentState == SERVICE_RUNNING || dwCurrentState == SERVICE_STOPPED) {
		service_status_.dwCheckPoint = 0;
	} else {
		service_status_.dwCheckPoint += 1;
	}

	::SetServiceStatus(service_status_handle_, &service_status_);
}

DWORD CServiceBase::UpdateControlsAccepted() {

	DWORD dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_POWEREVENT | SERVICE_ACCEPT_SESSIONCHANGE;
	if (!can_stop_) {
		dwControlsAccepted &= ~SERVICE_ACCEPT_STOP;
	}
	if (!can_pause_and_continue_) {
		dwControlsAccepted &= ~SERVICE_ACCEPT_PAUSE_CONTINUE;
	}
	if (!can_shutdown_) {
		dwControlsAccepted &= ~SERVICE_ACCEPT_SHUTDOWN;
	}
	if (!can_handle_power_event_) {
		dwControlsAccepted &= ~SERVICE_ACCEPT_POWEREVENT;
	}
	if (!can_handle_session_change_event_) {
		dwControlsAccepted &= ~SERVICE_ACCEPT_SESSIONCHANGE;
	}
	Utils::Output(Utils::StringFormat(L"CServiceBase::UpdateControlsAccepted dwControlsAccepted: %u", dwControlsAccepted));
	return dwControlsAccepted;
}

void CServiceBase::OnStart(DWORD dwNumServicesArgs, LPWSTR * lpServiceArgVectors) {
	Utils::Output(Utils::StringFormat(L"CServiceBase::OnStart dwNumServicesArgs: %u, lpServiceArgVectors: %p", dwNumServicesArgs, lpServiceArgVectors));
}

void CServiceBase::OnPause() {
	Utils::Output(L"CServiceBase::OnPause");
}

void CServiceBase::OnContinue() {
	Utils::Output(L"CServiceBase::OnContinue");
}

void CServiceBase::OnStop() {
	Utils::Output(L"CServiceBase::OnStop");
}

void CServiceBase::OnShutdown() {
	Utils::Output(L"CServiceBase::OnShutdown");
}

void CServiceBase::OnCustomCommand(DWORD command) {
	Utils::Output(Utils::StringFormat(L"CServiceBase::OnCustomCommand command: %u", command));
}

// 
// Event					Meaning
// 
// PBT_APMPOWERSTATUSCHANGE	Power status has changed.
// 10 (0xA)					��⵽�������Դ״̬�ĸ��ģ���ӵ�ص�Դ�л���������Դ�� ϵͳ������ʣ���ص��������û�ָ������ֵ����ʱ���ص����仯��ָ���İٷֱ�ʱ�㲥���¼���
// 
// PBT_APMRESUMEAUTOMATIC	Operation is resuming automatically from a low-power state. This message is sent every time the system resumes.
// 18 (0x12)				��������Զ������������¼��� ���ϵͳ�ڹ㲥 ResumeAutomatic ���⵽�κ��û�������㲥 ResumeSuspend �¼���ʹӦ�ó���֪�����ǿ��Իָ����û�����ȫ������
// 
// PBT_APMRESUMESUSPEND		Operation is resuming from a low-power state. This message is sent after PBT_APMRESUMEAUTOMATIC if the resume is triggered by user input, such as pressing a key.
// 7 (0x7)					ϵͳ�ڹ���֮���ѻָ�������
// 
// PBT_APMSUSPEND			System is suspending operation.
// 4 (0x4)					�������Ҫ�������״̬�� ���¼�ͨ��������Ӧ�ó���Ϳɰ�װ���������Ѷ���һ�� true ״̬���� QuerySuspend ��㲥��
// 
// PBT_POWERSETTINGCHANGE	A power setting change event has been received.
// 32787 (0x8013)
// 
void CServiceBase::OnPowerEvent(DWORD dwEventType, PPOWERBROADCAST_SETTING pSetting) {
	Utils::Output(Utils::StringFormat(L"CServiceBase::OnPowerEvent dwEventType: %u, pSetting: %p", dwEventType, pSetting));
}

// 
// Event						Meaning
// 
// WTS_CONSOLE_CONNECT			The session identified by lParam was connected to the console terminal or RemoteFX session.
// (0x1)						����̨�Ự�����ӡ�
// 
// WTS_CONSOLE_DISCONNECT		The session identified by lParam was disconnected from the console terminal or RemoteFX session.
// (0x2)						����̨�Ự�ѶϿ����ӡ�
// 
// WTS_REMOTE_CONNECT			The session identified by lParam was connected to the remote terminal.
// (0x3)						Զ�̻Ự�����ӡ�
// 
// WTS_REMOTE_DISCONNECT		The session identified by lParam was disconnected from the remote terminal.
// (0x4)						Զ�̻Ự�ѶϿ����ӡ�
// 
// WTS_SESSION_LOGON			A user has logged on to the session identified by lParam.
// (0x5)						�û��ѵ�¼���Ự��
// 
// WTS_SESSION_LOGOFF			A user has logged off the session identified by lParam.
// (0x6)						�û��ѴӻỰע����
// 
// WTS_SESSION_LOCK				The session identified by lParam has been locked.
// (0x7)						�Ự�ѱ�������
// 
// WTS_SESSION_UNLOCK			The session identified by lParam has been unlocked.
// (0x8)						�Ự�ѽ�����
// 
// WTS_SESSION_REMOTE_CONTROL	The session identified by lParam has changed its remote controlled status. To determine the status, call GetSystemMetrics and check the SM_REMOTECONTROL metric.
// (0x9)						�Ự��Զ�̿���״̬�Ѹ��ġ�
// 
// WTS_SESSION_CREATE			Reserved for future use.
// (0xA)
// 
// WTS_SESSION_TERMINATE		Reserved for future use.
// (0xB)
// 
void CServiceBase::OnSessionChange(DWORD dwEventType, PWTSSESSION_NOTIFICATION pNotification) {
	Utils::Output(Utils::StringFormat(L"CServiceBase::OnSessionChange dwEventType: %u, dwSessionId: %u", dwEventType, pNotification->dwSessionId));
}

}
