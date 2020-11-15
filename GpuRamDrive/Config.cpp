#include "stdafx.h"
#include "Config.h"
#include "GpuRamDrive.h"

Config::Config(LPCTSTR keyName)
{
	currentDeviceId = 0;
	version = 100;
	this->pszKeyName = new TCHAR[_tcslen(L"Software\\") + _tcslen(keyName) + 1];
	_tcscpy(this->pszKeyName, L"Software\\");
	_tcscat(this->pszKeyName, keyName);
#if GPU_API == GPU_API_CUDA
	_tcscat(this->pszKeyName, L"_CUDA");
#endif
	checkVersion();
}

Config::~Config()
{
}

void Config::checkVersion()
{
	DWORD configVersion = 0;
	getValue(L"Version", configVersion);
	if (configVersion == 0) {
		migrateVersion100();
	}
}

const std::vector<DWORD>& Config::getDeviceList()
{
	vectorDevices.clear();
	for (int c = 0; c <= 'Z' - 'A'; c++)
	{
		if (existValue(c, L"DriveLetter"))
			vectorDevices.push_back(c);
	}
	return vectorDevices;
}

BOOL Config::existDevice(DWORD deviceId)
{
	return existValue(deviceId, L"DriveLetter");
}

void Config::saveOriginalTempEnvironment()
{
	if (!existValue(L"OriginalTemp") || !existValue(L"OriginalTmp")) {
		wchar_t tempEnvironmentVariable[1024] = { 0 };
		wchar_t tmpEnvironmentVariable[1024] = { 0 };
		DWORD dwSize = obKey.GetSizeOfValue(_T("Environment"), L"TEMP", HKEY_CURRENT_USER);
		DWORD iTotalLength = dwSize + 1;
		memset(tempEnvironmentVariable, 0, iTotalLength);
		memset(tmpEnvironmentVariable, 0, iTotalLength);
		obKey.GetKeyValue(_T("Environment"), L"TEMP", tempEnvironmentVariable, iTotalLength);
		obKey.GetKeyValue(_T("Environment"), L"TMP", tmpEnvironmentVariable, iTotalLength);

		setValue(L"OriginalTemp", tempEnvironmentVariable);
		setValue(L"OriginalTmp", tmpEnvironmentVariable);
	}
}

void Config::setMountTempEnvironment(LPCTSTR pszValue)
{
	obKey.SetKeyValue(L"Environment", L"TEMP", pszValue, (DWORD)wcslen(pszValue) * 2, true, false, HKEY_CURRENT_USER);
	obKey.SetKeyValue(L"Environment", L"TMP", pszValue, (DWORD)wcslen(pszValue) * 2, true, false, HKEY_CURRENT_USER);
	SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
	SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)L"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
}

void Config::restoreOriginalTempEnvironment()
{
	wchar_t zsTemp[1024] = { 0 };
	if (getValue(L"OriginalTemp", zsTemp)) {
		obKey.SetKeyValue(L"Environment", L"TEMP", zsTemp, (DWORD)wcslen(zsTemp) * sizeof(wchar_t), true, false, HKEY_CURRENT_USER);
	}

	if (getValue(L"OriginalTmp", zsTemp)) {
		obKey.SetKeyValue(L"Environment", L"TMP", zsTemp, (DWORD)wcslen(zsTemp) * sizeof(wchar_t), true, false, HKEY_CURRENT_USER);
	}
	SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
	SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)L"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
}

DWORD Config::getCurrentDeviceId()
{
	return currentDeviceId;
}

void Config::setCurrentDeviceId(DWORD pszValue)
{
	currentDeviceId = pszValue;
}

DWORD Config::getGpuId()
{
	return getGpuId(currentDeviceId);
}

DWORD Config::getGpuId(DWORD deviceId)
{
	DWORD pszValue = 0;
	getValue(deviceId, L"GpuId", pszValue);
	return pszValue;
}

void Config::setGpuId(DWORD pszValue)
{
	setValue(currentDeviceId, L"GpuId", pszValue);
}

DWORD Config::getDriveLetter()
{
	return getDriveLetter(currentDeviceId);
}

DWORD Config::getDriveLetter(DWORD deviceId)
{
	DWORD pszValue = deviceId;
	getValue(deviceId, L"DriveLetter", pszValue);
	return pszValue;
}

void Config::setDriveLetter(DWORD pszValue)
{
	setValue(currentDeviceId, L"DriveLetter", pszValue);
}

DWORD Config::getDriveType()
{
	DWORD pszValue = 0;
	getValue(currentDeviceId, L"DriveType", pszValue);
	return pszValue;
}

void Config::setDriveType(DWORD pszValue)
{
	setValue(currentDeviceId, L"DriveType", pszValue);
}

DWORD Config::getDriveRemovable()
{
	DWORD pszValue = 0;
	getValue(currentDeviceId, L"DriveRemovable", pszValue);
	return pszValue;
}

void Config::setDriveRemovable(DWORD pszValue)
{
	setValue(currentDeviceId, L"DriveRemovable", pszValue);
}

DWORD Config::getDriveFormat()
{
	DWORD pszValue = 1;
	getValue(currentDeviceId, L"DriveFormat", pszValue);
	return pszValue;
}

void Config::setDriveFormat(DWORD pszValue)
{
	setValue(currentDeviceId, L"DriveFormat", pszValue);
}

DWORD Config::getMemSize()
{
	DWORD pszValue = 0;
	getValue(currentDeviceId, L"MemSize", pszValue);
	return pszValue;
}

void Config::setMemSize(DWORD pszValue)
{
	setValue(currentDeviceId, L"MemSize", pszValue);
}

void Config::getDriveLabel(LPTSTR pszValue)
{
	if (!getValue(currentDeviceId, L"DriveLabel", pszValue))
	{
		_tcscpy(pszValue, L"GpuDisk");
	}
}

void Config::setDriveLabel(LPCTSTR pszValue)
{
	setValue(currentDeviceId, L"DriveLabel", pszValue);
}

void Config::getImageFile(LPTSTR pszValue)
{
	if (!getValue(currentDeviceId, L"ImageFile", pszValue))
	{
		_tcscpy(pszValue, L"");
	}
}

void Config::setImageFile(LPCTSTR pszValue)
{
	setValue(currentDeviceId, L"ImageFile", pszValue);
}

DWORD Config::getReadOnly()
{
	DWORD pszValue = 0;
	getValue(currentDeviceId, L"ReadOnly", pszValue);
	return pszValue;
}

void Config::setReadOnly(DWORD pszValue)
{
	setValue(currentDeviceId, L"ReadOnly", pszValue);
}

DWORD Config::getTempFolder()
{
	DWORD pszValue = 0;
	getValue(currentDeviceId, L"TempFolder", pszValue);
	return pszValue;
}

void Config::setTempFolder(DWORD pszValue)
{
	setValue(currentDeviceId, L"TempFolder", pszValue);
}

DWORD Config::getStartOnWindows()
{
	return getStartOnWindows(currentDeviceId);
}

DWORD Config::getStartOnWindows(DWORD gpuId)
{
	DWORD pszValue = 0;
	getValue(gpuId, L"StartOnWindows", pszValue);
	return pszValue;
}

void Config::setStartOnWindows(DWORD pszValue)
{
	setValue(currentDeviceId, L"StartOnWindows", pszValue);
}

bool Config::getValue(LPCTSTR pszValueName, LPTSTR pszValue)
{
	DWORD dwSize = obKey.GetSizeOfValue(this->pszKeyName, pszValueName, HKEY_CURRENT_USER);
	if (dwSize != 0) {
		DWORD iTotalLength = dwSize + 1;
		memset(pszValue, 0, iTotalLength);
		return obKey.GetKeyValue(this->pszKeyName, pszValueName, pszValue, iTotalLength);
	}

	return false;
}

bool Config::getValue(DWORD gpu, LPCTSTR pszValueName, LPTSTR pszValue)
{
	LPTSTR keyName = new TCHAR[_tcslen(this->pszKeyName) + _tcslen(this->pszKeyName) + 3 + 1];
	_stprintf(keyName, _T("%s\\%d"), this->pszKeyName, gpu);

	DWORD dwSize = obKey.GetSizeOfValue(keyName, pszValueName, HKEY_CURRENT_USER);
	if (dwSize != 0) {
		DWORD iTotalLength = dwSize + 1;
		memset(pszValue, 0, iTotalLength);
		return obKey.GetKeyValue(keyName, pszValueName, pszValue, iTotalLength);
	}

	return false;
}

bool Config::getValue(LPCTSTR pszValueName, DWORD& dwValue)
{
	DWORD dwSize = obKey.GetSizeOfValue(this->pszKeyName, pszValueName, HKEY_CURRENT_USER);
	if (dwSize != 0) {
		return obKey.GetKeyValue(this->pszKeyName, pszValueName, dwValue);
	}

	return false;
}

bool Config::getValue(DWORD gpu, LPCTSTR pszValueName, DWORD& dwValue)
{
	LPTSTR keyName = new TCHAR[_tcslen(this->pszKeyName) + _tcslen(this->pszKeyName) + 3 + 1];
	_stprintf(keyName, _T("%s\\%d"), this->pszKeyName, gpu);

	DWORD dwSize = obKey.GetSizeOfValue(keyName, pszValueName, HKEY_CURRENT_USER);
	if (dwSize != 0) {
		return obKey.GetKeyValue(keyName, pszValueName, dwValue);
	}

	return false;
}

bool Config::setValue(LPCTSTR pszValueName, LPCTSTR pszValue)
{
	return obKey.SetKeyValue(this->pszKeyName, pszValueName, pszValue, (DWORD)wcslen(pszValue) * sizeof(LPCTSTR), true, false, HKEY_CURRENT_USER);
}

bool Config::setValue(DWORD gpu, LPCTSTR pszValueName, LPCTSTR pszValue)
{
	LPTSTR keyName = new TCHAR[_tcslen(this->pszKeyName) + _tcslen(this->pszKeyName) + 3 + 1];
	_stprintf(keyName, _T("%s\\%d"), this->pszKeyName, gpu);

	return obKey.SetKeyValue(keyName, pszValueName, pszValue, (DWORD)wcslen(pszValue) * sizeof(LPCTSTR), true, false, HKEY_CURRENT_USER);
}

bool Config::setValue(LPCTSTR pszValueName, DWORD pszValue)
{
	return obKey.SetKeyValue(this->pszKeyName, pszValueName, pszValue, true, false, HKEY_CURRENT_USER);
}

bool Config::setValue(DWORD gpu, LPCTSTR pszValueName, DWORD pszValue)
{
	LPTSTR keyName = new TCHAR[_tcslen(this->pszKeyName) + _tcslen(this->pszKeyName) + 3 + 1];
	_stprintf(keyName, _T("%s\\%d"), this->pszKeyName, gpu);

	return obKey.SetKeyValue(keyName, pszValueName, pszValue, true, false, HKEY_CURRENT_USER);
}

bool Config::existValue(LPCTSTR pszValueName)
{
	return obKey.GetSizeOfValue(pszKeyName, pszValueName, HKEY_CURRENT_USER) > 0;
}

bool Config::existValue(DWORD deviceId, LPCTSTR pszValueName)
{
	LPTSTR keyName = new TCHAR[_tcslen(this->pszKeyName) + _tcslen(this->pszKeyName) + 3 + 1];
	_stprintf(keyName, _T("%s\\%d"), this->pszKeyName, deviceId);

	return obKey.GetSizeOfValue(keyName, pszValueName, HKEY_CURRENT_USER) > 0;
}

bool Config::deleteDevice(DWORD deviceId)
{
	LPTSTR keyName = new TCHAR[_tcslen(this->pszKeyName) + _tcslen(this->pszKeyName) + 3 + 1];
	_stprintf(keyName, _T("%s\\%d"), this->pszKeyName, deviceId);

	return obKey.QuickDeleteKey(keyName, HKEY_CURRENT_USER);
}

bool Config::deleteValue(LPCTSTR pszValueName)
{
	return obKey.DeleteKeyValue(this->pszKeyName, pszValueName, HKEY_CURRENT_USER);
}

void Config::migrateVersion100()
{
	deleteValue(L"DefaultGpu");
	deleteDevice(0);
	deleteDevice(1);
	deleteDevice(2);
	deleteDevice(3);
	deleteDevice(4);
	deleteDevice(5);
	deleteDevice(6);
	deleteDevice(7);
	deleteDevice(8);
	deleteDevice(9);
	setValue(L"Version", version);
}
