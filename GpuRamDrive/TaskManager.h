#pragma once

#if !defined(_WINDOWS_)
#include <windows.h>
#endif

#include <tchar.h>

#include <comdef.h>
#include <taskschd.h>
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")


class TaskManager
{
private:

public:
	TaskManager();
	~TaskManager();

	bool ExistTaskJob(LPCTSTR pszTaskName);

	bool CreateTaskJob(LPCWSTR wszTaskName, wchar_t* nPath, wchar_t* nArguments);

	bool DeleteTaskJob(LPCWSTR wszTaskName);
};
