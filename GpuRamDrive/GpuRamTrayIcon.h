#pragma once

class GpuRamTrayIcon
{
private:
	NOTIFYICONDATA m_Data;
	std::wstring m_Tooltip;

public:
	GpuRamTrayIcon();
	~GpuRamTrayIcon();

	bool CreateIcon(HWND hWnd, HICON hIcon, UINT callbackMsg);
	bool Destroy();

	bool SetTooltip(const std::wstring& tooltip);
};

