#include "stdafx.h"
#include "Config.h"
#include "GpuRamDrive.h"

Config::Config(LPCTSTR keyName)
{
	currentGpu = 0;
	this->pszKeyName = new TCHAR[_tcslen(L"Software\\") + _tcslen(keyName) + 1];
	_tcscpy(this->pszKeyName, L"Software\\");
	_tcscat(this->pszKeyName, keyName);
#if GPU_API == GPU_API_CUDA
	_tcscat(this->pszKeyName, L"_CUDA");
#endif
}

Config::~Config()
{
}

void Config::deleteAllConfig(DWORD gpu)
{
	deleteValue(gpu, L"DriveLetter");
	deleteValue(gpu, L"DriveType");
	deleteValue(gpu, L"DriveRemovable");
	deleteValue(gpu, L"DriveFormat");
	deleteValue(gpu, L"MemSize");
	deleteValue(gpu, L"DriveLabel");
	deleteValue(gpu, L"TempFolder");
	deleteValue(gpu, L"StarOnWindows");
}

DWORD Config::getGpuList()
{
	DWORD pszValue = currentGpu;
	getValue(L"DefaultGpu", pszValue);
	return pszValue;
}

void Config::setGpuList(DWORD pszValue)
{
	setValue(L"DefaultGpu", pszValue);
	currentGpu = pszValue;
}

DWORD Config::getDriveLetter()
{
	DWORD pszValue = 'R' - 'A';
	getValue(currentGpu, L"DriveLetter", pszValue);
	return pszValue;
}

void Config::setDriveLetter(DWORD pszValue)
{
	setValue(currentGpu, L"DriveLetter", pszValue);
}

DWORD Config::getDriveType()
{
	DWORD pszValue = 0;
	getValue(currentGpu, L"DriveType", pszValue);
	return pszValue;
}

void Config::setDriveType(DWORD pszValue)
{
	setValue(currentGpu, L"DriveType", pszValue);
}

DWORD Config::getDriveRemovable()
{
	DWORD pszValue = 0;
	getValue(currentGpu, L"DriveRemovable", pszValue);
	return pszValue;
}

void Config::setDriveRemovable(DWORD pszValue)
{
	setValue(currentGpu, L"DriveRemovable", pszValue);
}

DWORD Config::getDriveFormat()
{
	DWORD pszValue = 1;
	getValue(currentGpu, L"DriveFormat", pszValue);
	return pszValue;
}

void Config::setDriveFormat(DWORD pszValue)
{
	setValue(currentGpu, L"DriveFormat", pszValue);
}

DWORD Config::getMemSize()
{
	DWORD pszValue = 0;
	getValue(currentGpu, L"MemSize", pszValue);
	return pszValue;
}

void Config::setMemSize(DWORD pszValue)
{
	setValue(currentGpu, L"MemSize", pszValue);
}

void Config::getDriveLabel(LPTSTR pszValue)
{
	if (!getValue(currentGpu, L"DriveLabel", pszValue))
	{
		_tcscpy(pszValue, L"GpuDisk");
	}
}

void Config::setDriveLabel(LPCTSTR pszValue)
{
	setValue(currentGpu, L"DriveLabel", pszValue);
}

DWORD Config::getTempFolder()
{
	DWORD pszValue = 0;
	getValue(currentGpu, L"TempFolder", pszValue);
	return pszValue;
}

void Config::setTempFolder(DWORD pszValue)
{
	setValue(currentGpu, L"TempFolder", pszValue);
}

DWORD Config::getStartOnWindows()
{
	DWORD pszValue = 0;
	getValue(currentGpu, L"StartOnWindows", pszValue);
	return pszValue;
}

void Config::setStartOnWindows(DWORD pszValue)
{
	setValue(currentGpu, L"StartOnWindows", pszValue);
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

bool Config::existValue(DWORD gpu, LPCTSTR pszValueName)
{
	LPTSTR keyName = new TCHAR[_tcslen(this->pszKeyName) + _tcslen(this->pszKeyName) + 3 + 1];
	_stprintf(keyName, _T("%s\\%d"), this->pszKeyName, gpu);

	return obKey.GetSizeOfValue(pszKeyName, pszValueName, HKEY_CURRENT_USER) > 0;
}

bool Config::deleteValue(DWORD gpu, LPCTSTR pszValueName)
{
	LPTSTR keyName = new TCHAR[_tcslen(this->pszKeyName) + _tcslen(this->pszKeyName) + 3 + 1];
	_stprintf(keyName, _T("%s\\%d"), this->pszKeyName, gpu);

	return obKey.DeleteKeyValue(keyName, pszValueName, HKEY_CURRENT_USER);
}

