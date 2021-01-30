#pragma once

#include "IWinlogon.h"
#include "Utils.h"
#include "UI/GinaWindow.h"
#include "MyService.h"

class Gina {
public:
    static BOOL Negotiate(DWORD dwWinlogonVersion, PDWORD pdwDllVersion);
    // ��ʼ��
    static BOOL Initialize(HANDLE hWlx, PVOID pWinlogonFunctions, Gina * * ppGina);

public:
    // δ��¼
    int LoggedOutSAS(DWORD dwSasType, PLUID pAuthenticationId, PSID pLogonSid, PDWORD pdwOptions, PHANDLE phToken, PWLX_MPR_NOTIFY_INFO pNprNotifyInfo, PVOID * ppWinlogonProfile);
    // �ѵ�¼
    int LoggedOnSAS(DWORD dwSasType);
    // ����
    int WkstaLockedSAS(DWORD dwSasType);

    BOOL ActivateUserShell(PWSTR pszDesktopName, PWSTR pszMprLogonScript, PVOID pEnvironment);

    // δ��¼ʱ��ʾctrl+del+alt�Ի���
    VOID DisplaySASNotice();
    // ����ʱ��ʾctrl+del+alt�Ի���
    VOID DisplayLockedNotice();

    BOOL IsLockOk();
    BOOL IsLogoffOk();

    // ע��
    VOID Logoff();
    // �ػ�
    VOID Shutdown(DWORD ShutdownType);

    BOOL ScreenSaverNotify(BOOL * pSecure);
    BOOL StartApplication(PWSTR pszDesktopName, PVOID pEnvironment, PWSTR pszCmdLine);

    BOOL NetworkProviderLoad(PWLX_MPR_NOTIFY_INFO pNprNotifyInfo);

    // ��ʾwindows״̬��Ϣ
    BOOL DisplayStatusMessage(HDESK hDesktop, DWORD dwOptions, PWSTR pTitle, PWSTR pMessage);
    BOOL GetStatusMessage(DWORD * pdwOptions, PWSTR pMessage, DWORD dwBufferSize);
    BOOL RemoveStatusMessage();

    BOOL GetConsoleSwitchCredentials(PWLX_CONSOLESWITCH_CREDENTIALS_INFO_V1_0 pCredInfo);
    VOID ReconnectNotify();
    VOID DisconnectNotify();

private:
    Gina(IWinlogon * pWinlogon, HANDLE LsaHandle);
    void Auth(Utils::Protocol::LoginRequest * request, Utils::Protocol::LoginResponse * response);

private:
    IWinlogon *             pWinlogon_;
    HANDLE                  hLsaHandle_;            // Pointer that receives a handle used in future authentication function calls.
    HANDLE                  hUserToken_;            // Handle of the users token.
    PWSTR                   pszProfilePath_;        // UNICODE_STRING specifying the path to the user's roaming profile if the user has a roaming profile. For example: \SomeServer\SomeShare\MyUserName
    GinaWindow *            pGinaWindow_;           // ��ʾ״̬��Ϣ����
    Utils::IService *       pService_;              // ҵ���߼��ӿ�
};
