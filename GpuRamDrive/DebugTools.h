#pragma once

class DebugTools
{
public:
	static void deb(wchar_t* msg, ...);
	static wchar_t* fmterr(DWORD err = GetLastError());
};