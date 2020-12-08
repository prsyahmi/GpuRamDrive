#pragma once
#include "Regkey.h"

class Config
{
private:
	xfc::RegKey obKey;
	LPTSTR pszKeyName;
	DWORD currentDeviceId;
	DWORD version;
	std::vector<DWORD> vectorDevices;

private:
	void checkVersion();
	void migrateVersion100();

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

	bool deleteValue(LPCTSTR pszValueName);

public:
	Config(LPCTSTR pszKeyName);
	~Config();

	const std::vector<DWORD>& getDeviceList();
	BOOL existDevice(DWORD deviceId);
	DWORD getDeviceTempFolfer();

	void saveOriginalTempEnvironment();
	void setMountTempEnvironment(LPCTSTR pszValue);
	void restoreOriginalTempEnvironment();

	DWORD getCurrentDeviceId();
	void setCurrentDeviceId(DWORD deviceId);

	DWORD getGpuId();
	DWORD getGpuId(DWORD deviceId);
	void setGpuId(DWORD pszValue);

	DWORD getDriveLetter();
	DWORD getDriveLetter(DWORD deviceId);
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

	DWORD getReadOnly();
	DWORD getReadOnly(DWORD deviceId);
	void setReadOnly(DWORD pszValue);

	DWORD getTempFolder();
	DWORD getTempFolder(DWORD deviceId);
	void setTempFolder(DWORD pszValue);

	DWORD getStartOnWindows();
	DWORD getStartOnWindows(DWORD deviceId);
	void setStartOnWindows(DWORD pszValue);

	bool deleteDevice(DWORD deviceId);
};