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

#define GPU_GUI_CLASS L"GPURAMDRIVE_CLASS"
#define SWM_TRAYINTERACTION    WM_APP + 1

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

std::wstring ToWide(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	return convert.from_bytes(str);
}


GpuRamGui::GpuRamGui()
{
	INITCOMMONCONTROLSEX c;
	c.dwSize = sizeof(c);
	c.dwICC = 0;
	wszTaskJobName = L"GPURAMDISK Task";

	InitCommonControlsEx(&c);
}


GpuRamGui::~GpuRamGui()
{
}

bool GpuRamGui::Create(HINSTANCE hInst, const std::wstring& title, int nCmdShow)
{
	m_Instance = hInst;
	SetProcessDPIAware();

	MyRegisterClass();

	m_hWnd = CreateWindowW(GPU_GUI_CLASS, title.c_str(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 700, 320, nullptr, nullptr, m_Instance, this);

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

void GpuRamGui::Mount(const std::wstring& device, size_t size, const std::wstring& driveLetter, const std::wstring& formatParam, const std::wstring& labelParam, const std::wstring& driveType, bool removable, bool tempFolderParam)
{
	bool found = false;
	int n = 0;
	size_t memSize = size * 1024 * 1024;

	auto vGpu = m_RamDrive.GetGpuDevices();
	for (auto it = vGpu.begin(); it != vGpu.end(); it++, n++) {
		if (ToWide(it->name).find(device) != std::string::npos) {
			found = true;
			break;
		}
	}

	if (!found) throw std::runtime_error("Unable to find device specified");

	ComboBox_SetCurSel(m_CtlGpuList, n);
	ComboBox_SetCurSel(m_CtlDriveLetter, (driveLetter[0] <= 'Z' ? driveLetter[0] - 'A' : driveLetter[0] - 'a'));
	ComboBox_SetCurSel(m_CtlDriveRemovable, (int)removable);
	Edit_SetText(m_CtlLabel, labelParam.c_str());
	Button_SetCheck(m_CtlTempFolder, tempFolderParam);

	ComboBox_SelectString(m_CtlFormatParam, -1, formatParam.c_str());
	for (int i = 0; i < ComboBox_GetCount(m_CtlFormatParam); i++) {
		wchar_t szItemName[128] = { 0 };
		ComboBox_GetLBText(m_CtlFormatParam, i, szItemName);
		wchar_t* wszFormatItem = _wcsdup(szItemName);
		_wcsupr_s(wszFormatItem, wcslen(wszFormatItem) + 1);

		wchar_t* szformatParam = _wcsdup(formatParam.c_str());
		_wcsupr_s(szformatParam, wcslen(szformatParam) + 1);

		if (wcsstr(szformatParam, szItemName) != 0) {
			ComboBox_SetCurSel(m_CtlFormatParam, i);
			break;
		}
	}

	wchar_t szTemp[64];
	_itow_s((int)size, szTemp, 10);
	Edit_SetText(m_CtlMemSize, szTemp);

	m_RamDrive.SetDriveType(driveType.c_str());
	m_RamDrive.SetRemovable(removable);
	m_RamDrive.CreateRamDevice(vGpu[n].platform_id, vGpu[n].device_id, L"GpuRamDev", memSize, driveLetter.c_str(), formatParam, labelParam, tempFolderParam);
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

	hStatic = CreateWindow(L"STATIC", L"Select Device:", WS_CHILD | WS_VISIBLE | SS_NOPREFIX, 10, 13, 140, 20, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(hStatic, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	hStatic = CreateWindow(L"STATIC", L"Drive Letter/Type:", WS_CHILD | WS_VISIBLE | SS_NOPREFIX, 10, 53, 140, 20, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(hStatic, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	hStatic = CreateWindow(L"STATIC", L"File System (MB):", WS_CHILD | WS_VISIBLE | SS_NOPREFIX, 10, 93, 140, 20, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(hStatic, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	hStatic = CreateWindow(L"STATIC", L"Volumen Label:", WS_CHILD | WS_VISIBLE | SS_NOPREFIX, 10, 133, 140, 20, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(hStatic, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlGpuList = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 150, 10, 150, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlGpuList, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlDriveLetter = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 150, 50, 150, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlDriveLetter, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlDriveType = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 315, 50, 150, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlDriveType, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlDriveRemovable = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 480, 50, 182, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlDriveRemovable, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlFormatParam = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 150, 90, 150, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlFormatParam, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlMemSize = CreateWindow(L"EDIT", L"1", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER, 315, 90, 150, 28, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlMemSize, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlLabel = CreateWindow(L"EDIT", L"GpuDisk", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP, 150, 130, 150, 28, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlLabel, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlTempFolder = CreateWindow(L"BUTTON", L"Create TEMP Folder", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX, 315, 132, 154, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlTempFolder, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlStartOnWindows = CreateWindow(L"BUTTON", L"Start on windows", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX, 520, 220, 154, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlStartOnWindows, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlMountBtn = CreateWindow(L"BUTTON", L"Mount", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 150, 190, 150, 40, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlMountBtn, WM_SETFONT, (WPARAM)FontBold, TRUE);

	wchar_t szTemp[64];
	wcscpy_s(szTemp, L"X:");
	for (wchar_t c = 'A'; c <= 'Z'; c++) {
		szTemp[0] = c;
		ComboBox_AddString(m_CtlDriveLetter, szTemp);
	}
	ComboBox_AddString(m_CtlDriveType, L"Hard Drive");
	ComboBox_AddString(m_CtlDriveType, L"Floppy Drive");
	ComboBox_AddString(m_CtlDriveRemovable, L"Non-Removable");
	ComboBox_AddString(m_CtlDriveRemovable, L"Removable");
	ComboBox_AddString(m_CtlFormatParam, L"FAT32");
	ComboBox_AddString(m_CtlFormatParam, L"exFAT");
	ComboBox_AddString(m_CtlFormatParam, L"NTFS");

	int suggestedGpuList = -1;
	int suggestedRamSize = 1;
	try
	{
		m_RamDrive.RefreshGPUInfo();
		auto v = m_RamDrive.GetGpuDevices();
		int index = 0;
		for (auto it = v.begin(); it != v.end(); it++, index++)
		{
			ComboBox_AddString(m_CtlGpuList, ToWide(it->name + " (" + std::to_string(it->memsize / (1024 * 1024)) + " MB)").c_str());
			if (suggestedGpuList == -1) {
#if GPU_API == GPU_API_HOSTMEM
				suggestedRamSize = 256;
#else
				suggestedRamSize = (int)((it->memsize / 1024 / 1024) - 1024);
#endif
			}

			if (it->name.find("GeForce") != std::string::npos || it->name.find("AMD") != std::string::npos) {
				suggestedRamSize = (int)((it->memsize / 1024 / 1024) - 1024);
				suggestedGpuList = index;
			}
		}
		if (suggestedGpuList == -1) {
			suggestedGpuList = ComboBox_GetCount(m_CtlGpuList) - 1;
		}
	}
	catch (const std::exception& ex)
	{
		ComboBox_AddString(m_CtlGpuList, ToWide(ex.what()).c_str());
	}

	ComboBox_SetCurSel(m_CtlGpuList, suggestedGpuList);
	ComboBox_SetCurSel(m_CtlDriveLetter, 'R' - 'A');
	ComboBox_SetCurSel(m_CtlDriveType, 0);
	ComboBox_SetCurSel(m_CtlDriveRemovable, 0);
	ComboBox_SetCurSel(m_CtlFormatParam, 1);
	Button_SetCheck(m_CtlTempFolder, TRUE);

	TaskManager taskManager;
	bool exists = taskManager.ExistTaskJob(wszTaskJobName);
	Button_SetCheck(m_CtlStartOnWindows, exists);

	wcscpy_s(szTemp, L"1");
	_itow_s(suggestedRamSize, szTemp, 10);
	Edit_SetText(m_CtlMemSize, szTemp);

	m_RamDrive.SetStateChangeCallback([&]() {
		m_UpdateState = true;
		InvalidateRect(m_hWnd, NULL, FALSE);

		if (m_RamDrive.IsMounted()) {
			m_Tray.CreateIcon(m_hWnd, m_Icon, SWM_TRAYINTERACTION);
			m_Tray.SetTooltip(L"GpuRamDrive");
		} else {
			m_Tray.Destroy();
		}
	});
	m_UpdateState = true;
}

void GpuRamGui::OnDestroy()
{
	PostQuitMessage(0);
}

void GpuRamGui::OnEndSession()
{
	m_RamDrive.ImdiskUnmountDevice();
}

void GpuRamGui::OnResize(WORD width, WORD height, bool minimized)
{
	MoveWindow(m_CtlGpuList, 150, 10, width - 150 - 20, 20, TRUE);
	MoveWindow(m_CtlMountBtn, width / 2 - 150, height - 90, 300, 70, TRUE);

	if (m_RamDrive.IsMounted() && minimized) {
		ShowWindow(m_hWnd, SW_HIDE);
	}
}

void GpuRamGui::OnMountClicked()
{
	if (!m_RamDrive.IsMounted())
	{
		auto vGpu = m_RamDrive.GetGpuDevices();
		int n = ComboBox_GetCurSel(m_CtlGpuList);

		if (n >= (int)vGpu.size()) {
			MessageBox(m_hWnd, L"GPU selection is invalid", L"Error while selecting GPU", MB_OK);
			return;
		}

		wchar_t szTemp[64] = { 0 };

		Edit_GetText(m_CtlMemSize, szTemp, sizeof(szTemp) / sizeof(wchar_t));
		size_t memSize = (size_t)_wtoi64(szTemp) * 1024 * 1024;

		ComboBox_GetText(m_CtlFormatParam, szTemp, sizeof(szTemp) / sizeof(wchar_t));
		wchar_t format[64] = { 0 };
		_snwprintf_s(format, sizeof(format), L"/fs:%s /q", szTemp);
		std::wstring formatParam = format;

		Edit_GetText(m_CtlLabel, szTemp, sizeof(szTemp) / sizeof(wchar_t));
		std::wstring labelParam = szTemp;

		bool tempFolderParam = Button_GetCheck(m_CtlTempFolder);

		EGpuRamDriveType driveType = ComboBox_GetCurSel(m_CtlDriveType) == 0 ? eGpuRamDriveType_HD : eGpuRamDriveType_FD;
		bool driveRemovable = ComboBox_GetCurSel(m_CtlDriveRemovable) == 0 ? false : true;

		if (memSize >= vGpu[n].memsize) {
			MessageBox(m_hWnd, L"The memory size you specified is too large", L"Invalid memory size", MB_OK);
			return;
		}

		ComboBox_GetText(m_CtlDriveLetter, szTemp, sizeof(szTemp) / sizeof(wchar_t));
		wchar_t* mountPointParam = szTemp;

		try
		{
			m_RamDrive.SetDriveType(driveType);
			m_RamDrive.SetRemovable(driveRemovable);
			m_RamDrive.CreateRamDevice(vGpu[n].platform_id, vGpu[n].device_id, L"GpuRamDev", memSize, mountPointParam, formatParam, labelParam, tempFolderParam);
		}
		catch (const std::exception& ex)
		{
			MessageBoxA(m_hWnd, ex.what(), "Error while mounting GPU Ram Drive", MB_OK);
		}
	}
	else
	{
		m_RamDrive.ImdiskUnmountDevice();
	}
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

	if (m_RamDrive.IsMounted())
	{
		EnableWindow(m_CtlDriveLetter, FALSE);
		EnableWindow(m_CtlDriveType, FALSE);
		EnableWindow(m_CtlDriveRemovable, FALSE);
		EnableWindow(m_CtlGpuList, FALSE);
		EnableWindow(m_CtlMemSize, FALSE);
		EnableWindow(m_CtlLabel, FALSE);
		EnableWindow(m_CtlFormatParam, FALSE);
		EnableWindow(m_CtlTempFolder, FALSE);
		EnableWindow(m_CtlStartOnWindows, FALSE);
		Edit_SetText(m_CtlMountBtn, L"Unmount");
	}
	else
	{
		EnableWindow(m_CtlDriveLetter, TRUE);
		EnableWindow(m_CtlDriveType, TRUE);
		EnableWindow(m_CtlDriveRemovable, TRUE);
		EnableWindow(m_CtlGpuList, TRUE);
		EnableWindow(m_CtlMemSize, TRUE);
		EnableWindow(m_CtlLabel, TRUE);
		EnableWindow(m_CtlFormatParam, TRUE);
		EnableWindow(m_CtlTempFolder, TRUE);
		EnableWindow(m_CtlStartOnWindows, TRUE);
		Edit_SetText(m_CtlMountBtn, L"Mount");
	}

	m_UpdateState = false;
}

ATOM GpuRamGui::MyRegisterClass()
{
	WNDCLASSEXW wcex;

	m_Icon = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_ICON1));

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
			}
			break;
		}

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

		case WM_COMMAND:
		{
			if (_this) {
				if ((HANDLE)lParam == _this->m_CtlMountBtn) {
					EnableWindow(_this->m_CtlMountBtn, FALSE);
					_this->OnMountClicked();
					EnableWindow(_this->m_CtlMountBtn, TRUE);
				}
				else if ((HANDLE)lParam == _this->m_CtlGpuList) {
					if (HIWORD(wParam) == CBN_SELCHANGE) {

						int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

						TGPUDevice it = _this->m_RamDrive.GetGpuDevices().at(ItemIndex);
						int suggestedRamSize = (int)(it.memsize * 0.8 / 1024 / 1024);

						wchar_t szTemp[64];
						wcscpy_s(szTemp, L"1");
						_itow_s(suggestedRamSize, szTemp, 10);
						Edit_SetText(_this->m_CtlMemSize, szTemp);
					}
				}
				else if ((HANDLE)lParam == _this->m_CtlTempFolder) {
					BOOL checked = Button_GetCheck(_this->m_CtlTempFolder);
					Button_SetCheck(_this->m_CtlTempFolder, !checked);
				}
				else if ((HANDLE)lParam == _this->m_CtlStartOnWindows) {
					BOOL checked = Button_GetCheck(_this->m_CtlStartOnWindows);
					Button_SetCheck(_this->m_CtlStartOnWindows, !checked);

					TaskManager taskManager;

					if (!checked) {
						wchar_t nPath[MAX_PATH];
						GetModuleFileName(NULL, nPath, MAX_PATH);
						wchar_t nArguments[MAX_PATH * 4] = { 0 };

						wchar_t szTemp[64] = { 0 };
						wchar_t szDevice[64] = { 0 };
						ComboBox_GetText(_this->m_CtlGpuList, szTemp, sizeof(szTemp) / sizeof(wchar_t));
						size_t pos = std::wstring(szTemp).find_first_of(L" ");
						wcsncpy(szDevice, szTemp, pos);

						wchar_t szMemSize[64] = { 0 };
						Edit_GetText(_this->m_CtlMemSize, szMemSize, sizeof(szMemSize) / sizeof(wchar_t));

						wchar_t szFormat[10] = { 0 };
						ComboBox_GetText(_this->m_CtlFormatParam, szFormat, sizeof(szFormat) / sizeof(wchar_t));

						wchar_t szLabel[64] = { 0 };
						Edit_GetText(_this->m_CtlLabel, szLabel, sizeof(szLabel) / sizeof(wchar_t));

						bool tempFolderParam = Button_GetCheck(_this->m_CtlTempFolder);

						EGpuRamDriveType driveType = ComboBox_GetCurSel(_this->m_CtlDriveType) == 0 ? eGpuRamDriveType_HD : eGpuRamDriveType_FD;
						bool driveRemovable = ComboBox_GetCurSel(_this->m_CtlDriveRemovable) == 0 ? false : true;

						wchar_t szMountPoint[10] = { 0 };
						ComboBox_GetText(_this->m_CtlDriveLetter, szMountPoint, sizeof(szMountPoint) / sizeof(wchar_t));

						_snwprintf_s(nArguments, sizeof(nArguments), L"--device %s --size %s --mount %s --format %s --label %s %s --hide",
							szDevice, szMemSize, szMountPoint, szFormat, szLabel, tempFolderParam ? L"--temp_folder" : L"");
						taskManager.CreateTaskJob(_this->wszTaskJobName, nPath, nArguments);
					}
					else {
						taskManager.DeleteTaskJob(_this->wszTaskJobName);
					}

					/*
					xfc::RegKey obKey;
					LPCTSTR keyName = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
					LPCTSTR valueName = L"GPURAMDRIVE";

					if (!checked) {
						wchar_t nPath[MAX_PATH];
						GetModuleFileName(NULL, nPath, MAX_PATH);
						wchar_t value[MAX_PATH * 4] = { 0 };

						wchar_t szTemp[64] = { 0 };
						wchar_t szDevice[64] = { 0 };
						ComboBox_GetText(_this->m_CtlGpuList, szTemp, sizeof(szTemp) / sizeof(wchar_t));
						size_t pos = std::wstring(szTemp).find_first_of(L" ");
						wcsncpy(szDevice, szTemp, pos);

						wchar_t szMemSize[64] = { 0 };
						Edit_GetText(_this->m_CtlMemSize, szMemSize, sizeof(szMemSize) / sizeof(wchar_t));

						wchar_t szFormat[10] = { 0 };
						ComboBox_GetText(_this->m_CtlFormatParam, szFormat, sizeof(szFormat) / sizeof(wchar_t));

						wchar_t szLabel[64] = { 0 };
						Edit_GetText(_this->m_CtlLabel, szLabel, sizeof(szLabel) / sizeof(wchar_t));

						bool tempFolderParam = Button_GetCheck(_this->m_CtlTempFolder);

						EGpuRamDriveType driveType = ComboBox_GetCurSel(_this->m_CtlDriveType) == 0 ? eGpuRamDriveType_HD : eGpuRamDriveType_FD;
						bool driveRemovable = ComboBox_GetCurSel(_this->m_CtlDriveRemovable) == 0 ? false : true;

						wchar_t szMountPoint[10] = { 0 };
						ComboBox_GetText(_this->m_CtlDriveLetter, szMountPoint, sizeof(szMountPoint) / sizeof(wchar_t));

						_snwprintf_s(value, sizeof(value), L"\"%s\" --device %s --size %s --mount %s --format %s --label %s %s --hide",
							nPath, szDevice, szMemSize, szMountPoint, szFormat, szLabel, tempFolderParam ? L"--temp_folder" : L"");

						obKey.SetKeyValue(keyName, valueName, value, (DWORD)wcslen(value) * sizeof(wchar_t), true, false, HKEY_CURRENT_USER);
					}
					else {
						obKey.QuickDeleteKey(keyName, HKEY_CURRENT_USER);
					}
					*/
				}
			}
			break;
		}

		case SWM_TRAYINTERACTION:
			if (_this) _this->OnTrayInteraction(lParam);
			break;

		default:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	return 0;
}
