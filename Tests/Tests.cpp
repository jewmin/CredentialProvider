// Tests.cpp : ���ļ����� "main" ����������ִ�н��ڴ˴���ʼ��������
//

#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <wincred.h>
#include "Utils.h"
#include "FileLog.h"

int main()
{
	// BOOL save = false;
	// DWORD authPackage = 0;
	// LPVOID authBuffer;
	// ULONG authBufferSize = 0;
	// CREDUI_INFO credUiInfo;

	// credUiInfo.pszCaptionText = TEXT("My caption");
	// credUiInfo.pszMessageText = TEXT("My message");
	// credUiInfo.cbSize = sizeof(credUiInfo);
	// credUiInfo.hbmBanner = NULL;
	// credUiInfo.hwndParent = NULL;

	// CredUIPromptForWindowsCredentials(&credUiInfo, 0, &authPackage, NULL, 0, &authBuffer, &authBufferSize, &save, 0);

	Utils::CFileLog log;
	Utils::SetLog(&log);
	Utils::Output(L"test����123");
	log.SetLogFileNameFormat(L"%d%d%d.log");
	Utils::Output(L"sksksjdf���Կ�Ҷsdfk��");
	Utils::WToA(NULL);
	Utils::StringFormat(L"����abc %s %d", L"����", 1000);
	return 0;
}

// ���г���: Ctrl + F5 ����� >����ʼִ��(������)���˵�
// ���Գ���: F5 ����� >����ʼ���ԡ��˵�

// ������ʾ: 
//   1. ʹ�ý��������Դ�������������/�����ļ�
//   2. ʹ���Ŷ���Դ�������������ӵ�Դ�������
//   3. ʹ��������ڲ鿴���������������Ϣ
//   4. ʹ�ô����б��ڲ鿴����
//   5. ת������Ŀ��>���������Դ����µĴ����ļ�����ת������Ŀ��>�����������Խ����д����ļ���ӵ���Ŀ
//   6. ��������Ҫ�ٴδ򿪴���Ŀ����ת�����ļ���>���򿪡�>����Ŀ����ѡ�� .sln �ļ�
