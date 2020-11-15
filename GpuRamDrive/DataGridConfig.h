#pragma once
#if !defined(_WINDOWS_)
#include <windows.h>
#endif
#include "DataGrid.h"
#include "Config.h"

class DataGridConfig
{
private:
	CDataGrid m_hdataGrid;

private:
	void add(Config config, DWORD deviceId);
	void setRowColor(DWORD deviceId);

public:
	DataGridConfig();
	~DataGridConfig();

	void create(HWND hwnd, RECT rect);
	HWND getDataGridHandler();
	void sendWinProcEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void reload(Config config);
	DWORD getSelectedDeviceId();
};