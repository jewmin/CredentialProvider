// Tests.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <credentialprovider.h>
#include <wincred.h>
#include "Utils.h"
#include "FileLog.h"

#define enum2string(val) L#val

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
	Utils::Output(L"test测试123");
	log.SetLogFileNameFormat(L"%d%d%d.log");
	Utils::Output(L"sksksjdf测试枯叶sdfk夺");
	Utils::WToA(NULL);
	Utils::StringFormat(L"测试abc %s %d", L"瞧瞧", 1000);
	// Utils::Output(enum2string(CPUS_LOGON));
	// CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus = CPUS_UNLOCK_WORKSTATION;
	// Utils::Output(enum2string(CREDENTIAL_PROVIDER_USAGE_SCENARIO(cpus)));
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
