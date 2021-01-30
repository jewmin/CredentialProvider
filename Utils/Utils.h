#pragma once

#include "ILog.h"

namespace Utils {

// ������־
void SetLog(ILog * log);
// д��־
void Output(const std::wstring & message);
// ��ʽ���ַ���
std::wstring StringFormat(const wchar_t * format, ...);
std::wstring StringFormatVa(const wchar_t * format, va_list args);
// ��ʽ��Windows����
std::wstring GetLastErrorString();
std::wstring GetErrorString(DWORD error_code);

// ���ֽ�ת���ֽ�
std::string WToA(const std::wstring & input);
std::string WToA(const wchar_t * input);
std::string WToA(const wchar_t * input, const size_t len);

// ���ֽ�ת���ֽ�
std::wstring AToW(const std::string & input);
std::wstring AToW(const char * input);
std::wstring AToW(const char * input, const size_t len);

}
