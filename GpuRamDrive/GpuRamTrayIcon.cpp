#include "stdafx.h"
#include "GpuRamTrayIcon.h"

// Using GUID will causes an issue, see all the details here: https://github.com/electron/electron/pull/2882
static const GUID TRAY_GUID = { 0xf5b95301, 0xd87e, 0x45fd, {0x96, 0xfb, 0xad, 0x4f, 0xfd, 0x08, 0x1d, 0xeb} };

GpuRamTrayIcon::GpuRamTrayIcon()
{
	memset(&m_Data, 0, sizeof(m_Data));
	m_Data.cbSize = sizeof(m_Data);
	m_Data.guidItem = TRAY_GUID;
}


GpuRamTrayIcon::~GpuRamTrayIcon()
{
	Destroy();
}

bool GpuRamTrayIcon::CreateIcon(HWND hWnd, HICON hIcon, UINT callbackMsg)
{
	m_Data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP/* | NIF_GUID*/;
	m_Data.hWnd = hWnd;
	wcsncpy_s(m_Data.szTip, ARRAYSIZE(m_Data.szTip), m_Tooltip.c_str(), min(ARRAYSIZE(m_Data.szTip), m_Tooltip.length()));
	m_Data.hIcon = hIcon;
	m_Data.uID = 1;
	m_Data.uCallbackMessage = callbackMsg;

	return Shell_NotifyIcon(NIM_ADD, &m_Data) > 0;
}

bool GpuRamTrayIcon::Destroy()
{
	return Shell_NotifyIcon(NIM_DELETE, &m_Data) > 0;
}

bool GpuRamTrayIcon::SetTooltip(const std::wstring& tooltip)
{
	m_Tooltip = tooltip;
	wcsncpy_s(m_Data.szTip, ARRAYSIZE(m_Data.szTip), m_Tooltip.c_str(), min(ARRAYSIZE(m_Data.szTip), m_Tooltip.length()));

	return Shell_NotifyIcon(NIM_MODIFY, &m_Data) > 0;
}
