#pragma once
#if !defined(_WINDOWS_)
#include <windows.h>
#endif
#include <map>
#include "GPURamDrive.h"
#include "DataGrid.h"
#include "Config.h"

class DataGridConfig
{
private:
	CDataGrid m_hdataGrid;

private:
	void add(Config config, DWORD deviceId, bool isMounted);

public:
	DataGridConfig();
	~DataGridConfig();

	void create(HWND hwnd, RECT rect);
	HWND getDataGridHandler();
	void sendWinProcEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void reload(Config config, std::map<DWORD, GPURamDrive> &m_RamDrive);
	DWORD getSelectedDeviceId();
	void setRowMount(DWORD deviceId, BOOL value);
	void resetSelection();
};