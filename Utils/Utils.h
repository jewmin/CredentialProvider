#pragma once

#include "ILog.h"

namespace Utils {

void SetLog(ILog * log);
void Output(const std::wstring & message);
std::wstring StringFormat(const wchar_t * format, ...);
std::wstring StringFormatVa(const wchar_t * format, va_list args);
std::wstring GetLastErrorString();

std::string WToA(const std::wstring & input);
std::string WToA(const wchar_t * input);
std::string WToA(const wchar_t * input, const size_t len);

std::wstring AToW(const std::string & input);
std::wstring AToW(const char * input);
std::wstring AToW(const char * input, const size_t len);

}
