/*
GpuRamDrive proxy for ImDisk Virtual Disk Driver.

Copyright (C) 2016 Syahmi Azhar.

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "stdafx.h"
#include "GpuRamDrive.h"
#include "GpuRamGui.h"
#include "resource.h"
#include "TaskManager.h"
#include "DiskUtil.h"
#include "DebugTools.h"


#define GPU_GUI_CLASS L"GPURAMDRIVE_CLASS"
#define SWM_TRAYINTERACTION    WM_APP + 1
#define IDT_TIMER1 1001

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

std::wstring ToWide(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	return convert.from_bytes(str);
}


GpuRamGui::GpuRamGui()
	: m_Icon(NULL)
	, m_IconMounted(NULL)
	, m_Instance(NULL)
	, m_hWnd(NULL)
	, m_AutoMount(false)
	, m_CtlGpuList(NULL)
	, m_CtlMountBtn(NULL)
	, m_CtlMemSize(NULL)
	, m_CtlDriveLetter(NULL)
	, m_CtlDriveType(NULL)
	, m_CtlDriveRemovable(NULL)
	, m_CtlDriveLabel(NULL)
	, m_CtlDriveFormat(NULL)
	, m_CtlImageFile(NULL)
	, m_CtlChooseFileBtn(NULL)
	, m_CtlReadOnly(NULL)
	, m_CtlTempFolder(NULL)
	, m_CtlStartOnWindows(NULL)
	, m_CtlAddDeviceBtn(NULL)
	, m_CtlModifyDeviceBtn(NULL)
	, m_CtlRemoveDeviceBtn(NULL)
	, m_UpdateState(false)
	, wszAppName(L"GpuRamDrive")
	, wszTaskJobName(L"GPURAMDRIVE Task")
	, diskUtil()
	, config(wszAppName)
	, dataGridConfig()
{
	INITCOMMONCONTROLSEX c;
	c.dwSize = sizeof(c);
	c.dwICC = 0;

	InitCommonControlsEx(&c);
	DebugTools::deb(L"Started %s", wszAppName);
}


GpuRamGui::~GpuRamGui()
{
	DebugTools::deb(L"Closed %s", wszAppName);
}

bool GpuRamGui::Create(HINSTANCE hInst, const std::wstring& title, int nCmdShow, bool autoMount)
{
	m_Instance = hInst;
	m_AutoMount = autoMount;
	SetProcessDPIAware();

	MyRegisterClass();

	m_hWnd = CreateWindowW(GPU_GUI_CLASS, title.c_str(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 700, 570, nullptr, nullptr, m_Instance, this);

	if (!m_hWnd) return false;

	ShowWindow(m_hWnd, nCmdShow);
	UpdateWindow(m_hWnd);

	return m_hWnd != NULL;
}

int GpuRamGui::Loop()
{
	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

void GpuRamGui::AutoMount()
{
	if (m_AutoMount)
	{
		auto devices = config.getDeviceList();
		for (int i = 0; i < devices.size(); i++)
		{
			RestoreGuiParams(devices.at(i), 256);
			if (config.getStartOnWindows() == 1)
			{
				OnMountClicked(devices.at(i));
				m_Tray.SetTooltip(wszAppName, true);
			}
		}
	}
}

void GpuRamGui::RestoreWindow()
{
	ShowWindow(m_hWnd, SW_RESTORE);
	SetForegroundWindow(m_hWnd);
}

void GpuRamGui::OnCreate()
{
	SetWindowLongPtr(m_hWnd, GWL_STYLE, GetWindowLongPtr(m_hWnd, GWL_STYLE) & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX);

	HFONT FontBold = CreateFontA(-18, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE, "Segoe UI");
	HFONT FontNormal = CreateFontA(-15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE, "Segoe UI");
	HWND hStatic;

	hStatic = CreateWindow(L"STATIC", L"Select Gpu:", WS_CHILD | WS_VISIBLE | SS_NOPREFIX, 10, 13, 140, 20, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(hStatic, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	hStatic = CreateWindow(L"STATIC", L"Drive Letter/Type:", WS_CHILD | WS_VISIBLE | SS_NOPREFIX, 10, 53, 140, 20, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(hStatic, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	hStatic = CreateWindow(L"STATIC", L"Format/MB/Label:", WS_CHILD | WS_VISIBLE | SS_NOPREFIX, 10, 93, 140, 20, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(hStatic, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	hStatic = CreateWindow(L"STATIC", L"Image File:", WS_CHILD | WS_VISIBLE | SS_NOPREFIX, 10, 133, 140, 20, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(hStatic, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	hStatic = CreateWindow(L"STATIC", L"Other options:", WS_CHILD | WS_VISIBLE | SS_NOPREFIX, 10, 173, 140, 20, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(hStatic, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlGpuList = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 150, 10, 150, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlGpuList, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlDriveLetter = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 150, 50, 150, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlDriveLetter, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlDriveType = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 315, 50, 150, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlDriveType, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlDriveRemovable = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 480, 50, 182, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlDriveRemovable, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlDriveFormat = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 150, 90, 150, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlDriveFormat, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlMemSize = CreateWindow(L"EDIT", L"1", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER, 315, 90, 150, 28, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlMemSize, WM_SETFONT, (WPARAM)FontNormal, TRUE);
	SendMessage(m_CtlMemSize, EM_SETCUEBANNER, 0, LPARAM(L"Mem size (MB)"));

	m_CtlDriveLabel = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP, 480, 90, 182, 28, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlDriveLabel, WM_SETFONT, (WPARAM)FontNormal, TRUE);
	SendMessage(m_CtlDriveLabel, EM_SETCUEBANNER, 0, LPARAM(L"Volumen label"));

	m_CtlImageFile = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP, 150, 130, 315, 28, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlImageFile, WM_SETFONT, (WPARAM)FontNormal, TRUE);
	SendMessage(m_CtlImageFile, EM_SETCUEBANNER, 0, LPARAM(L"Image file"));

	m_CtlChooseFileBtn = CreateWindow(L"BUTTON", L"...", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 480, 130, 25, 28, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlChooseFileBtn, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlReadOnly = CreateWindow(L"BUTTON", L"Read only", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX, 515, 132, 154, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlReadOnly, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlTempFolder = CreateWindow(L"BUTTON", L"Create TEMP Folder", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX, 150, 170, 154, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlTempFolder, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlStartOnWindows = CreateWindow(L"BUTTON", L"Start on windows", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX, 315, 170, 154, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlStartOnWindows, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlAddDeviceBtn = CreateWindow(L"BUTTON", L"Add Device", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, 210, 150, 28, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlAddDeviceBtn, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlModifyDeviceBtn = CreateWindow(L"BUTTON", L"Modify Device", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 265, 210, 150, 28, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlModifyDeviceBtn, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlRemoveDeviceBtn = CreateWindow(L"BUTTON", L"Remove Device", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 514, 210, 150, 28, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlRemoveDeviceBtn, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlMountBtn = CreateWindow(L"BUTTON", L"Mount", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 250, 190, 150, 40, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlMountBtn, WM_SETFONT, (WPARAM)FontBold, TRUE);

	RECT rect = { 10, 240, 10 + 655, 240 + 165 };
	dataGridConfig.create(m_hWnd, rect);

	ReloadDriveLetterList();

	ComboBox_AddString(m_CtlDriveType, L"Hard Drive");
	ComboBox_AddString(m_CtlDriveType, L"Floppy Drive");
	ComboBox_AddString(m_CtlDriveRemovable, L"Non-Removable");
	ComboBox_AddString(m_CtlDriveRemovable, L"Removable");
	ComboBox_AddString(m_CtlDriveFormat, L"FAT32");
	ComboBox_AddString(m_CtlDriveFormat, L"exFAT");
	ComboBox_AddString(m_CtlDriveFormat, L"NTFS");

	int suggestedRamSize = -1;
	try
	{
		auto v = m_RamDrive[0].GetGpuDevices();
		int index = 0;
		for (auto it = v.begin(); it != v.end(); it++, index++)
		{
			ComboBox_AddString(m_CtlGpuList, ToWide(std::to_string(index) + ": " + it->name + " (" + std::to_string(it->memsize / (1024 * 1024)) + " MB)").c_str());
			if (suggestedRamSize == -1) {
#if GPU_API == GPU_API_HOSTMEM
				suggestedRamSize = 256;
#else
				suggestedRamSize = (int)((it->memsize / 1024 / 1024) - 1024);
#endif
			}

			m_RamDrive[index].SetStateChangeCallback([&]() {
				m_UpdateState = true;
				InvalidateRect(m_hWnd, NULL, FALSE);
				});
		}

		if (config.getCurrentDeviceId() >= index)
		{
			config.setCurrentDeviceId(0);
		}

		auto devices = config.getDeviceList();
		if (devices.size() > 0) {
			RestoreGuiParams(devices.at(0), suggestedRamSize);
		}
		else {
			RestoreGuiParams('R' - 'A', suggestedRamSize);
		}
		dataGridConfig.reload(config, m_RamDrive);
	}
	catch (const std::exception& ex)
	{
		ComboBox_AddString(m_CtlGpuList, ToWide(ex.what()).c_str());
	}

	m_UpdateState = true;
	AutoMount();
}

bool GpuRamGui::CreateTryIcon()
{
	bool result = m_Tray.CreateIcon(m_hWnd, m_Icon, m_IconMounted, SWM_TRAYINTERACTION);
	if (result)
	{
		m_Tray.SetTooltip(wszAppName, m_AutoMount);
		return true;
	}
	SetTimer(m_hWnd, IDT_TIMER1, 1000, (TIMERPROC)NULL);
	return false;
}

void GpuRamGui::ReloadDriveLetterList()
{
	ComboBox_ResetContent(m_CtlDriveLetter);
	wchar_t szTemp[64];
	wcscpy_s(szTemp, L"X:");
	for (wchar_t c = 'A'; c <= 'Z'; c++)
	{
		wchar_t szTemp2[64];
		_snwprintf_s(szTemp, sizeof(szTemp), (diskUtil.checkDriveIsMounted(c, szTemp2) ? L"%c: - %s" : L"%c:%s"), c, szTemp2);
		ComboBox_AddString(m_CtlDriveLetter, szTemp);
	}
	ComboBox_SetCurSel(m_CtlDriveLetter, config.getDriveLetter());

	m_UpdateState = true;
	UpdateState();
}

boolean GpuRamGui::IsAnyMounted()
{
	auto devices = config.getDeviceList();
	for (int i = 0; i < devices.size(); i++)
	{
		if(m_RamDrive[devices.at(i)].IsMounted())
			return true;
	}

	return false;
}

void GpuRamGui::RestoreGuiParams(DWORD deviceId, DWORD suggestedRamSize)
{
	if (deviceId != (DWORD)-1) {
		config.setCurrentDeviceId(deviceId);
		ComboBox_SetCurSel(m_CtlGpuList, config.getGpuId());
		ComboBox_SetCurSel(m_CtlDriveLetter, config.getDriveLetter());
		ComboBox_SetCurSel(m_CtlDriveType, config.getDriveType());
		ComboBox_SetCurSel(m_CtlDriveRemovable, config.getDriveRemovable());
		ComboBox_SetCurSel(m_CtlDriveFormat, config.getDriveFormat());
		Button_SetCheck(m_CtlReadOnly, config.getReadOnly());
		Button_SetCheck(m_CtlTempFolder, config.getTempFolder());
		Button_SetCheck(m_CtlStartOnWindows, config.getStartOnWindows());

		wchar_t szTemp[1024] = {};
		wcscpy_s(szTemp, L"1");
		if (config.getMemSize() > 0) {
			_itow_s(config.getMemSize(), szTemp, 10);
		}
		else {
			_itow_s(suggestedRamSize, szTemp, 10);
		}
		Edit_SetText(m_CtlMemSize, szTemp);

		config.getDriveLabel(szTemp);
		Edit_SetText(m_CtlDriveLabel, szTemp);

		config.getImageFile(szTemp);
		Edit_SetText(m_CtlImageFile, szTemp);
	}
	m_UpdateState = true;
	UpdateState();
}

void GpuRamGui::SaveGuiParams()
{
	config.setCurrentDeviceId(ComboBox_GetCurSel(m_CtlDriveLetter));
	config.setGpuId(ComboBox_GetCurSel(m_CtlGpuList));
	config.setDriveLetter(ComboBox_GetCurSel(m_CtlDriveLetter));
	config.setDriveType(ComboBox_GetCurSel(m_CtlDriveType));
	config.setDriveRemovable(ComboBox_GetCurSel(m_CtlDriveRemovable));
	config.setDriveFormat(ComboBox_GetCurSel(m_CtlDriveFormat));
	config.setMemSize(ComboBox_GetCurSel(m_CtlDriveFormat));
	config.setReadOnly(Button_GetCheck(m_CtlReadOnly));
	config.setTempFolder(Button_GetCheck(m_CtlTempFolder));
	config.setStartOnWindows(Button_GetCheck(m_CtlStartOnWindows));

	wchar_t szTemp[64] = { 0 };
	Edit_GetText(m_CtlMemSize, szTemp, sizeof(szTemp) / sizeof(wchar_t));
	config.setMemSize((size_t)_wtoi64(szTemp));

	Edit_GetText(m_CtlDriveLabel, szTemp, sizeof(szTemp) / sizeof(wchar_t));
	config.setDriveLabel(szTemp);

	Edit_GetText(m_CtlImageFile, szTemp, sizeof(szTemp) / sizeof(wchar_t));
	config.setImageFile(szTemp);

	SetStartOnWindows();

	dataGridConfig.reload(config, m_RamDrive);
	m_UpdateState = true;
	UpdateState();
}

void GpuRamGui::RemoveDevice(DWORD deviceId)
{
	if (deviceId != (DWORD)-1) {
		config.deleteDevice(deviceId);
		dataGridConfig.reload(config, m_RamDrive);
		m_UpdateState = true;
		UpdateState();
	}
}

void GpuRamGui::SetStartOnWindows()
{
	TaskManager taskManager;

	auto devices = config.getDeviceList();
	boolean isStartOnWindows = false;
	for (int i = 0; i < devices.size(); i++)
	{
		isStartOnWindows = config.getStartOnWindows(devices.at(i));
		if (isStartOnWindows)
			break;
	}

	if (isStartOnWindows) {
		wchar_t nPath[MAX_PATH] = {};
		GetModuleFileName(NULL, nPath, MAX_PATH);
		taskManager.CreateTaskJob(wszTaskJobName, nPath, L"--autoMount --hide");
	}
	else
	{
		taskManager.DeleteTaskJob(wszTaskJobName);
	}
}

void GpuRamGui::OnDestroy()
{
	OnEndSession();
	PostQuitMessage(0);
}

void GpuRamGui::OnEndSession()
{
	auto devices = config.getDeviceList();
	for (int i = 0; i < devices.size(); i++)
	{
		if (m_RamDrive[devices.at(i)].IsMounted())
			m_RamDrive[devices.at(i)].ImdiskUnmountDevice();
	}
}

void GpuRamGui::OnResize(WORD width, WORD height, bool minimized)
{
	MoveWindow(m_CtlGpuList, 150, 10, width - 150 - 20, 20, TRUE);
	MoveWindow(m_CtlMountBtn, width / 2 - 150, height - 90, 300, 70, TRUE);

	if (IsAnyMounted() && minimized) {
		ShowWindow(m_hWnd, SW_HIDE);
	}
}

void GpuRamGui::OnMountClicked(DWORD deviceId)
{
	if (deviceId == (DWORD)-1) {
		return;
	}

	int gpuId = ComboBox_GetCurSel(m_CtlGpuList);
	auto vGpu = m_RamDrive[0].GetGpuDevices();
	if (gpuId >= (int)vGpu.size()) {
		MessageBox(m_hWnd, L"GPU selection is invalid", L"Error while selecting GPU", MB_OK);
		return;
	}

	bool isMounted = m_RamDrive[deviceId].IsMounted();
	if (!isMounted)
	{
		wchar_t szTemp[64] = { 0 };

		Edit_GetText(m_CtlMemSize, szTemp, sizeof(szTemp) / sizeof(wchar_t));
		size_t memSize = (size_t)_wtoi64(szTemp) * 1024 * 1024;

		ComboBox_GetText(m_CtlDriveFormat, szTemp, sizeof(szTemp) / sizeof(wchar_t));
		wchar_t format[64] = { 0 };
		_snwprintf_s(format, sizeof(format), L"/fs:%s /q", szTemp);
		std::wstring formatParam = format;

		Edit_GetText(m_CtlDriveLabel, szTemp, sizeof(szTemp) / sizeof(wchar_t));
		std::wstring labelParam = szTemp;

		bool tempFolderParam = Button_GetCheck(m_CtlTempFolder);

		EGpuRamDriveType driveType = ComboBox_GetCurSel(m_CtlDriveType) == 0 ? EGpuRamDriveType::HD : EGpuRamDriveType::FD;
		bool driveRemovable = ComboBox_GetCurSel(m_CtlDriveRemovable) == 0 ? false : true;

		if (memSize >= vGpu[gpuId].memsize) {
			MessageBox(m_hWnd, L"The memory size you specified is too large", L"Invalid memory size", MB_OK);
			return;
		}

		ComboBox_GetText(m_CtlDriveLetter, szTemp, sizeof(szTemp) / sizeof(wchar_t));
		wchar_t* mountPointParam = szTemp;

		if (diskUtil.checkDriveIsMounted(mountPointParam[0], NULL))
		{
			MessageBox(m_hWnd, L"It is not possible to mount the unit, it is already in use", wszAppName, MB_OK);
			return;
		}

		try
		{
			m_RamDrive[deviceId].SetDriveType(driveType);
			m_RamDrive[deviceId].SetRemovable(driveRemovable);
			m_RamDrive[deviceId].CreateRamDevice(vGpu[gpuId].platform_id, vGpu[gpuId].device_id, L"GpuRamDev_" + std::to_wstring(mountPointParam[0]), memSize, mountPointParam, formatParam, labelParam, tempFolderParam);
			dataGridConfig.setRowMount(deviceId, true);

			dataGridConfig.resetSelection();
			try
			{
				wchar_t szImageFile[MAX_PATH] = { 0 };
				config.getImageFile(szImageFile);
				if (wcslen(szImageFile) > 0 && diskUtil.fileExists(szImageFile)) {
					DebugTools::deb(L"Restoring the image '%s'", szImageFile);
					wchar_t szDeviceVolumen[MAX_PATH] = { 0 };
					_snwprintf_s(szDeviceVolumen, sizeof(szDeviceVolumen), L"\\\\.\\%s", mountPointParam);
					diskUtil.restore(szImageFile, szDeviceVolumen);
				}
			}
			catch (const std::exception& ex)
			{
				MessageBoxA(m_hWnd, ex.what(), "Error restoring the image file", MB_OK);
			}
		}
		catch (const std::exception& ex)
		{
			MessageBoxA(m_hWnd, ex.what(), "Error while mounting drive", MB_OK);
		}
	}
	else
	{
		try
		{
			if (!Button_GetCheck(m_CtlReadOnly))
			{
				wchar_t szImageFile[MAX_PATH] = { 0 };
				config.getImageFile(szImageFile);
				if (wcslen(szImageFile) > 0) {
					wchar_t szTemp[64] = { 0 };
					ComboBox_GetText(m_CtlDriveLetter, szTemp, sizeof(szTemp) / sizeof(wchar_t));

					wchar_t szDeviceVolumen[MAX_PATH] = { 0 };
					_snwprintf_s(szDeviceVolumen, sizeof(szDeviceVolumen), L"\\\\.\\%s", szTemp);

					diskUtil.save(szDeviceVolumen, szImageFile);
				}
			}
		}
		catch (const std::exception& ex)
		{
			MessageBoxA(m_hWnd, ex.what(), "Error saving the image file", MB_OK);
		}
		m_RamDrive[deviceId].ImdiskUnmountDevice();
		dataGridConfig.setRowMount(deviceId, false);
	}
	ReloadDriveLetterList();
	if (isMounted)
	{
		SendMessage(dataGridConfig.getDataGridHandler(), WM_LBUTTONDOWN, (WPARAM)NULL, MAKELPARAM(275, 50));
		SendMessage(dataGridConfig.getDataGridHandler(), WM_LBUTTONUP, (WPARAM)NULL, MAKELPARAM(275, 50));
	}
	else
	{
		SendMessage(dataGridConfig.getDataGridHandler(), WM_LBUTTONDOWN, MAKEWPARAM(0, 0), (LPARAM)NULL);
		SendMessage(dataGridConfig.getDataGridHandler(), WM_LBUTTONUP, MAKEWPARAM(0, 0), (LPARAM)NULL);
	}
	RestoreGuiParams((DWORD)-1, 0);
}

void GpuRamGui::OnTrayInteraction(LPARAM lParam)
{
	switch (lParam)
	{
		case WM_LBUTTONUP:
			if (IsWindowVisible(m_hWnd) && !IsIconic(m_hWnd)) {
				ShowWindow(m_hWnd, SW_HIDE);
			} else {
				ShowWindow(m_hWnd, SW_RESTORE);
				SetForegroundWindow(m_hWnd);
			}
			break;
		case WM_RBUTTONUP:
		case WM_CONTEXTMENU:
			break;
	}
}

void GpuRamGui::UpdateState()
{
	if (!m_UpdateState) return;

	DWORD deviceId = dataGridConfig.getSelectedDeviceId();
	EnableWindow(m_CtlGpuList, TRUE);
	EnableWindow(m_CtlDriveLetter, TRUE);
	EnableWindow(m_CtlDriveType, TRUE);
	EnableWindow(m_CtlDriveRemovable, TRUE);
	EnableWindow(m_CtlDriveFormat, TRUE);
	EnableWindow(m_CtlMemSize, TRUE);
	EnableWindow(m_CtlDriveLabel, TRUE);
	EnableWindow(m_CtlImageFile, TRUE);
	EnableWindow(m_CtlChooseFileBtn, TRUE);
	EnableWindow(m_CtlReadOnly, TRUE);
	EnableWindow(m_CtlTempFolder, TRUE);
	EnableWindow(m_CtlStartOnWindows, TRUE);
	EnableWindow(m_CtlAddDeviceBtn, TRUE);
	if (deviceId != (DWORD)-1) {
		EnableWindow(m_CtlAddDeviceBtn, FALSE);
		EnableWindow(m_CtlModifyDeviceBtn, TRUE);
		EnableWindow(m_CtlRemoveDeviceBtn, TRUE);
		EnableWindow(m_CtlMountBtn, TRUE);
	}
	else
	{
		EnableWindow(m_CtlAddDeviceBtn, TRUE);
		EnableWindow(m_CtlModifyDeviceBtn, FALSE);
		EnableWindow(m_CtlRemoveDeviceBtn, FALSE);
		EnableWindow(m_CtlMountBtn, FALSE);
	}
	Edit_SetText(m_CtlMountBtn, L"Mount");

	if (deviceId != (DWORD)-1 && m_RamDrive[deviceId].IsMounted())
	{
		EnableWindow(m_CtlGpuList, FALSE);
		EnableWindow(m_CtlDriveLetter, FALSE);
		EnableWindow(m_CtlDriveType, FALSE);
		EnableWindow(m_CtlDriveRemovable, FALSE);
		EnableWindow(m_CtlDriveFormat, FALSE);
		EnableWindow(m_CtlMemSize, FALSE);
		EnableWindow(m_CtlDriveLabel, FALSE);
		EnableWindow(m_CtlImageFile, FALSE);
		EnableWindow(m_CtlChooseFileBtn, FALSE);
		EnableWindow(m_CtlReadOnly, FALSE);
		EnableWindow(m_CtlTempFolder, FALSE);
		EnableWindow(m_CtlStartOnWindows, FALSE);
		EnableWindow(m_CtlAddDeviceBtn, FALSE);
		EnableWindow(m_CtlModifyDeviceBtn, FALSE);
		EnableWindow(m_CtlRemoveDeviceBtn, FALSE);
		EnableWindow(m_CtlMountBtn, TRUE);
		Edit_SetText(m_CtlMountBtn, L"Unmount");
	}

	m_UpdateState = false;

	if (IsAnyMounted())
	{
		SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)m_IconMounted);
		SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)m_IconMounted);
		m_Tray.SetTooltip(wszAppName, true);
	}
	else
	{
		SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)m_Icon);
		SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)m_Icon);
		m_Tray.SetTooltip(wszAppName, false);
	}
}

ATOM GpuRamGui::MyRegisterClass()
{
	WNDCLASSEXW wcex;

	m_Icon = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_ICON1));
	m_IconMounted = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_ICON2));

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = m_Instance;
	wcex.hIcon = m_Icon;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = GPU_GUI_CLASS;
	wcex.hIconSm = wcex.hIcon;

	return RegisterClassExW(&wcex);
}

LRESULT CALLBACK GpuRamGui::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	GpuRamGui* _this = (GpuRamGui*)(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message)
	{
		case WM_CREATE:
		{
			LPCREATESTRUCTW pCreateParam = (LPCREATESTRUCTW)lParam;
			if (pCreateParam) {
				_this = (GpuRamGui*)pCreateParam->lpCreateParams;
				SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreateParam->lpCreateParams);
			}

			if (_this) {
				_this->m_hWnd = hWnd;
				_this->OnCreate();
				_this->CreateTryIcon();
			}
			break;
		}
		case WM_CLOSE:
		{
			if (!_this->m_AutoMount && _this->IsAnyMounted())
			{
				if (MessageBox(hWnd, L"The drive is mounted, do you really want to exit?", _this->wszAppName, MB_OKCANCEL) == IDOK)
					DestroyWindow(hWnd);
			}
			else
				DestroyWindow(hWnd);
		}
		break;
		case WM_TIMER:
		{
			if (_this->CreateTryIcon())
				KillTimer(_this->m_hWnd, IDT_TIMER1);
		}
		break;
		case WM_ENDSESSION:
			if (_this) _this->OnEndSession();
			break;

		case WM_DESTROY:
			if (_this) _this->OnDestroy();
			break;

		case WM_SIZE:
			if (_this) _this->OnResize(LOWORD(lParam), HIWORD(lParam), wParam == SIZE_MINIMIZED);
			break;

		case WM_PAINT:
			_this->UpdateState();
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;

		case WM_SYSCOMMAND:
			if ((wParam & 0xFFF0) == SC_MINIMIZE)
			{
				if (_this)
				{
					_this->OnTrayInteraction(WM_LBUTTONUP);
				}
				break;
			}
			else
			{
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		case SWM_TRAYINTERACTION:
			if (_this) _this->OnTrayInteraction(lParam);
			break;

		case WM_COMMAND:
			if (_this) {
				if ((HANDLE)lParam == _this->dataGridConfig.getDataGridHandler()) {
					DWORD deviceId = _this->dataGridConfig.getSelectedDeviceId();
					_this->RestoreGuiParams(deviceId, 0);
				}
				
				if ((HANDLE)lParam == _this->m_CtlMountBtn) {
					EnableWindow(_this->m_CtlMountBtn, FALSE);
					_this->OnMountClicked(_this->dataGridConfig.getSelectedDeviceId());
					//EnableWindow(_this->m_CtlMountBtn, TRUE);
				}
				else if ((HANDLE)lParam == _this->m_CtlDriveLetter) {
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						LRESULT letterIndex = SendMessage((HWND)_this->m_CtlDriveLetter, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
						LRESULT gpuIndex = SendMessage((HWND)_this->m_CtlGpuList, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
						TGPUDevice it = _this->m_RamDrive[0].GetGpuDevices().at(gpuIndex);
#if GPU_API == GPU_API_HOSTMEM
						int suggestedRamSize = 256;
#else
						int suggestedRamSize = (int)((it.memsize / 1024 / 1024) - 1024);
#endif
						_this->RestoreGuiParams(letterIndex, suggestedRamSize);
					}
				}
				if ((HANDLE)lParam == _this->m_CtlAddDeviceBtn) {
					DWORD deviceId = ComboBox_GetCurSel(_this->m_CtlDriveLetter);
					if (_this->config.existDevice(deviceId)) {
						MessageBox(hWnd, L"The letter is already added, select a free letter to add a new device", _this->wszAppName, MB_OK);
						return false;
					}
					if (_this->diskUtil.checkDriveIsMounted('A' + deviceId, NULL)) {
						MessageBox(hWnd, L"The letter is in use, select a free letter to add a new device", _this->wszAppName, MB_OK);
						return false;
					}
					_this->SaveGuiParams();
				}
				if ((HANDLE)lParam == _this->m_CtlModifyDeviceBtn) {
					DWORD deviceId = ComboBox_GetCurSel(_this->m_CtlDriveLetter);
					DWORD selectedDeviceId = _this->dataGridConfig.getSelectedDeviceId();
					if (selectedDeviceId != deviceId && _this->config.existDevice(deviceId)) {
						MessageBox(hWnd, L"The letter is already added, select a free letter to modify the device", _this->wszAppName, MB_OK);
						return false;
					}
					if (_this->diskUtil.checkDriveIsMounted('A' + deviceId, NULL)) {
						MessageBox(hWnd, L"The letter is in use, select a free letter to modify the device", _this->wszAppName, MB_OK);
						return false;
					}
					_this->config.deleteDevice(selectedDeviceId);
					_this->SaveGuiParams();
				}
				if ((HANDLE)lParam == _this->m_CtlRemoveDeviceBtn) {
					DWORD deviceId = _this->dataGridConfig.getSelectedDeviceId();
					_this->RemoveDevice(deviceId);
				}
				else if ((HANDLE)lParam == _this->m_CtlReadOnly) {
					BOOL checked = Button_GetCheck(_this->m_CtlReadOnly);
					Button_SetCheck(_this->m_CtlReadOnly, !checked);
				}
				else if ((HANDLE)lParam == _this->m_CtlTempFolder) {
					BOOL checked = !Button_GetCheck(_this->m_CtlTempFolder);
					DWORD deviceId = _this->dataGridConfig.getSelectedDeviceId();
					DWORD tempFolderDeviceId = _this->config.getDeviceTempFolfer();

					if (checked && tempFolderDeviceId != (DWORD)-1 && deviceId != tempFolderDeviceId)
					{
						MessageBox(hWnd, L"The temp folder is already selected on another device, it can only be selected once", _this->wszAppName, MB_OK);
						return false;
					}

					Button_SetCheck(_this->m_CtlTempFolder, checked);
					if (checked) {
						_this->config.saveOriginalTempEnvironment();
					}
				}
				else if ((HANDLE)lParam == _this->m_CtlStartOnWindows) {
					BOOL checked = Button_GetCheck(_this->m_CtlStartOnWindows);
					Button_SetCheck(_this->m_CtlStartOnWindows, !checked);
				}
				else if ((HANDLE)lParam == _this->m_CtlChooseFileBtn) {
					std::wstring file = _this->diskUtil.chooserFile(L"Select the image file", L"(*.img) Image file\0*.img\0");
					if (file.length() > 0) {
						Edit_SetText(_this->m_CtlImageFile, file.c_str());
					}
				}
				else {
					return DefWindowProc(hWnd, message, wParam, lParam);
				}
			}
			break;
		default:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	return 0;
}
