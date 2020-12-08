#pragma once
#if !defined(_WINDOWS_)
#include <windows.h>
#endif

#include "Regkey.h"

class DiskUtil
{
private:
	LPVOID buffer;
	HANDLE hinput, houtput;
	xfc::RegKey obKey;

public:
	DiskUtil();
	~DiskUtil();

	BOOL checkDriveIsMounted(char letter, wchar_t* type);
	BOOL fileExists(LPCTSTR szPath);
	std::wstring chooserFile(const wchar_t* title, const wchar_t* filter);
	DWORD save(const wchar_t* source_device_name, const wchar_t* filename);
	DWORD restore(const wchar_t* filename, const wchar_t* target_device_name);
	void createDriveIcon(char letter);
	void removeDriveIcon(char letter);

private:
	void throwError(const char* text);
	void freeResources();

};