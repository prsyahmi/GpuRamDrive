#include "stdafx.h"
#include "DataGridConfig.h"

DataGridConfig::DataGridConfig()
{
}

DataGridConfig::~DataGridConfig()
{
}

void DataGridConfig::create(HWND hwnd, RECT rect)
{
	m_hdataGrid.Create(rect, hwnd, 8);
	// Set DataGrid column info
	m_hdataGrid.SetColumnInfo(0, L"Letter", 70, DGTA_CENTER);
	m_hdataGrid.SetColumnInfo(1, L"GpuId", 70, DGTA_CENTER);
	m_hdataGrid.SetColumnInfo(2, L"Format", 80, DGTA_CENTER);
	m_hdataGrid.SetColumnInfo(3, L"MemSize", 100, DGTA_CENTER);
	m_hdataGrid.SetColumnInfo(4, L"Label", 100, DGTA_CENTER);
	m_hdataGrid.SetColumnInfo(5, L"Temp", 70, DGTA_CENTER);
	m_hdataGrid.SetColumnInfo(6, L"Start", 70, DGTA_CENTER);
	m_hdataGrid.SetColumnInfo(7, L"Mount", 80, DGTA_CENTER);

	ShowScrollBar(m_hdataGrid.GetWindowHandle(), SB_BOTH, FALSE );

	LOGFONT lf;
	m_hdataGrid.GetColumnFont(&lf);
	_tcscpy(lf.lfFaceName, _T("Segoe UI"));
	m_hdataGrid.SetColumnFont(&lf);

	m_hdataGrid.GetRowFont(&lf);
	_tcscpy(lf.lfFaceName, _T("Segoe UI"));
	m_hdataGrid.SetRowFont(&lf);
}

HWND DataGridConfig::getDataGridHandler()
{
	return m_hdataGrid.GetSafeHwnd();
}

void DataGridConfig::sendWinProcEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	m_hdataGrid.SubEditProc(hWnd, message, wParam, lParam);
}

void DataGridConfig::add(Config config, DWORD deviceId)
{
	int rowNumber = m_hdataGrid.GetRowNumber();
	config.setCurrentDeviceId(deviceId);

	TCHAR szItem[MAX_PATH];
	_stprintf(szItem, _T("%c:"), 'A' + deviceId);
	m_hdataGrid.InsertItem(szItem, DGTA_CENTER);
	m_hdataGrid.SetItemInfo(rowNumber, 0, szItem, DGTA_CENTER, true, deviceId);

	_stprintf(szItem, _T("%d"), config.getGpuId());
	m_hdataGrid.SetItemInfo(rowNumber, 1, szItem, DGTA_CENTER, true);

	if (config.getDriveFormat() == 0)
		m_hdataGrid.SetItemInfo(rowNumber, 2, L"FAT32", DGTA_CENTER, true);
	else if (config.getDriveFormat() == 1)
		m_hdataGrid.SetItemInfo(rowNumber, 2, L"exFAT", DGTA_CENTER, true);
	else
		m_hdataGrid.SetItemInfo(rowNumber, 2, L"NTFS", DGTA_CENTER, true);

	_stprintf(szItem, _T("%d"), config.getMemSize());
	m_hdataGrid.SetItemInfo(rowNumber, 3, szItem, DGTA_CENTER, true);

	config.getDriveLabel(szItem);
	m_hdataGrid.SetItemInfo(rowNumber, 4, szItem, DGTA_CENTER, true);

	if (config.getTempFolder())
		m_hdataGrid.SetItemInfo(rowNumber, 5, L"True", DGTA_CENTER, true);
	else
		m_hdataGrid.SetItemInfo(rowNumber, 5, L"False", DGTA_CENTER, true);

	if (config.getStartOnWindows())
		m_hdataGrid.SetItemInfo(rowNumber, 6, L"True", DGTA_CENTER, true);
	else
		m_hdataGrid.SetItemInfo(rowNumber, 6, L"False", DGTA_CENTER, true);

	m_hdataGrid.SetItemInfo(rowNumber, 7, L"Mount", DGTA_CENTER, true);

	setRowColor(rowNumber);
}

void DataGridConfig::remove(Config config, DWORD deviceId)
{
	config.deleteDevice(deviceId);
	m_hdataGrid.RemoveAllItems();
	auto v = config.getDeviceList();
	for (int i = 0; i < v.size(); i++)
	{
		add(config, v.at(i));
	}
}

void DataGridConfig::setRowColor(DWORD deviceId) {
	int iLeft = deviceId % 3;
	switch (iLeft)
	{
	case 0:
		m_hdataGrid.SetItemBgColor(deviceId, RGB(250, 220, 220));
		m_hdataGrid.SetItemBgColor(deviceId, RGB(220, 250, 220));
		break;
	case 1:
		m_hdataGrid.SetItemBgColor(deviceId, RGB(220, 250, 220));
		break;
	case 2:
		m_hdataGrid.SetItemBgColor(deviceId, RGB(250, 250, 220));
		break;
	}
	m_hdataGrid.Update();
}
