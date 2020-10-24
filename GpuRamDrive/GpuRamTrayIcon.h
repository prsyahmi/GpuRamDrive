#pragma once

class GpuRamTrayIcon
{
private:
	NOTIFYICONDATA m_Data;
	wchar_t m_Tooltip[256] = {};

public:
	GpuRamTrayIcon();
	~GpuRamTrayIcon();

	bool CreateIcon(HWND hWnd, HICON hIcon, UINT callbackMsg);
	bool Destroy();

	bool SetTooltip(const std::wstring& tooltip, char driveLetter);
};

