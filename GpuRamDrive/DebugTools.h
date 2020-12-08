#pragma once

class DebugTools
{
private:
	LPTSTR pszFileName;

public:
	DebugTools(LPCTSTR pszFileName);
	~DebugTools();
	void deb(wchar_t* msg, ...);
	wchar_t* fmterr(DWORD err = GetLastError());
	void writeToFile(LPCTSTR filename, wchar_t* data);
	std::string getCurrentTimestamp();
};