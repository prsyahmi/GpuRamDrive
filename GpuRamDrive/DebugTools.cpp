#include "stdafx.h"
#include "DebugTools.h"

void DebugTools::deb(wchar_t* msg, ...)
{
	va_list ap;
	wchar_t string[2048] = {};
	wchar_t stringout[4096] = {};

	va_start(ap, msg);
	vswprintf(string, wcslen(string) - 1, msg, ap);
	va_end(ap);

	swprintf(stringout, wcslen(stringout) - 1,  L"<%X> %s\n", GetCurrentThreadId(), string);
	OutputDebugString(stringout);
}

wchar_t* DebugTools::fmterr(DWORD err)
{
	LPVOID lpMsgBuf = NULL;
	static wchar_t szInternal[255] = { 0 };

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	lstrcpy(szInternal, (wchar_t*)lpMsgBuf);
	LocalFree(lpMsgBuf);
	return szInternal;
}