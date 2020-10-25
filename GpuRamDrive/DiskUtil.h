#pragma once
#if !defined(_WINDOWS_)
#include <windows.h>
#endif

class DiskUtil
{
private:
	LPVOID buffer;
	HANDLE hinput, houtput;

public:
	DiskUtil();
	~DiskUtil();

	BOOL checkDriveIsMounted(char letter, wchar_t* type);
	BOOL fileExists(LPCTSTR szPath);
	std::wstring chooserFile(const wchar_t* title, const wchar_t* filter);
	DWORD save(const wchar_t* source_device_name, const wchar_t* filename);
	DWORD restore(const wchar_t* filename, const wchar_t* target_device_name);

private:
	void throwError(const char* text);
	void freeResources();

};