#pragma once
#if !defined(_WINDOWS_)
#include <windows.h>
#endif
#include "Regkey.h"

class Config
{
private:
	xfc::RegKey obKey;
	LPTSTR pszKeyName;
	DWORD currentGpu;

private:
	bool getValue(LPCTSTR pszValueName, LPTSTR pszValue);
	bool getValue(DWORD gpu, LPCTSTR pszValueName, LPTSTR pszValue);

	bool getValue(LPCTSTR pszValueName, DWORD& dwValue);
	bool getValue(DWORD gpu, LPCTSTR pszValueName, DWORD& dwValue);

	bool setValue(LPCTSTR pszValueName, DWORD pszValue);
	bool setValue(DWORD gpu, LPCTSTR pszValueName, DWORD pszValue);

	bool setValue(LPCTSTR pszValueName, LPCTSTR pszValue);
	bool setValue(DWORD gpu, LPCTSTR pszValueName, LPCTSTR pszValue);

	bool existValue(LPCTSTR pszValueName);
	bool existValue(DWORD gpu, LPCTSTR pszValueName);

	bool deleteValue(DWORD gpu, LPCTSTR pszValueName);

public:
	Config(LPCTSTR pszKeyName);
	~Config();

	void deleteAllConfig(DWORD gpu);

	void saveOriginalTempEnvironment();
	void setMountTempEnvironment(LPCTSTR pszValue);
	void restoreOriginalTempEnvironment();

	DWORD getGpuList();
	void setGpuList(DWORD pszValue);

	DWORD getDriveLetter();
	void setDriveLetter(DWORD pszValue);

	DWORD getDriveType();
	void setDriveType(DWORD pszValue);

	DWORD getDriveRemovable();
	void setDriveRemovable(DWORD pszValue);

	DWORD getDriveFormat();
	void setDriveFormat(DWORD pszValue);

	DWORD getMemSize();
	void setMemSize(DWORD pszValue);

	void getDriveLabel(LPTSTR pszValue);
	void setDriveLabel(LPCTSTR pszValue);

	void getImageFile(LPTSTR pszValue);
	void setImageFile(LPCTSTR pszValue);

	DWORD getTempFolder();
	void setTempFolder(DWORD pszValue);

	DWORD getStartOnWindows();
	void setStartOnWindows(DWORD pszValue);
};