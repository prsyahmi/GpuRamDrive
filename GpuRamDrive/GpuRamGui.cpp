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
	: m_Icon(NULL)
	, m_Instance(NULL)
	, m_hWnd(NULL)
	, m_CtlGpuList(NULL)
	, m_CtlMountBtn(NULL)
	, m_CtlMemSize(NULL)
	, m_CtlDriveLetter(NULL)
	, m_CtlDriveType(NULL)
	, m_CtlDriveRemovable(NULL)
	, m_CtlDriveLabel(NULL)
	, m_CtlDriveFormat(NULL)
	, m_CtlTempFolder(NULL)
	, m_CtlStartOnWindows(NULL)
	, m_UpdateState(false)
	, wszAppName(L"GpuRamDrive")
	, wszTaskJobName(L"GPURAMDRIVE Task")
	, config(wszAppName)
{
	INITCOMMONCONTROLSEX c;
	c.dwSize = sizeof(c);
	c.dwICC = 0;

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

void GpuRamGui::Mount(DWORD gpu)
{
	m_RamDrive.RefreshGPUInfo();
	if (gpu >= m_RamDrive.GetGpuDevices().size()) throw std::runtime_error("Unable to find device specified");
	RestoreGuiParams(gpu, 256);
	OnMountClicked();
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

	m_CtlDriveFormat = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 150, 90, 150, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlDriveFormat, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlMemSize = CreateWindow(L"EDIT", L"1", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER, 315, 90, 150, 28, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlMemSize, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlDriveLabel = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP, 150, 130, 150, 28, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlDriveLabel, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlTempFolder = CreateWindow(L"BUTTON", L"Create TEMP Folder", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX, 315, 132, 154, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlTempFolder, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlStartOnWindows = CreateWindow(L"BUTTON", L"Start on windows", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX, 520, 220, 154, 25, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlStartOnWindows, WM_SETFONT, (WPARAM)FontNormal, TRUE);

	m_CtlMountBtn = CreateWindow(L"BUTTON", L"Mount", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 150, 190, 150, 40, m_hWnd, NULL, m_Instance, NULL);
	SendMessage(m_CtlMountBtn, WM_SETFONT, (WPARAM)FontBold, TRUE);

	wchar_t szTemp[64];
	wcscpy_s(szTemp, L"X:");
	for (wchar_t c = 'A'; c <= 'Z'; c++)
	{
		wchar_t szTemp2[64];
		_snwprintf_s(szTemp, sizeof(szTemp), (CheckDriveIsMounted(c, szTemp2) ? L"%c: - %s" : L"%c:%s"), c, szTemp2);
		ComboBox_AddString(m_CtlDriveLetter, szTemp);
	}
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
		m_RamDrive.RefreshGPUInfo();
		auto v = m_RamDrive.GetGpuDevices();
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
		}

		if (config.getGpuList() >= index)
		{
			config.setGpuList(0);
		}
	}
	catch (const std::exception& ex)
	{
		ComboBox_AddString(m_CtlGpuList, ToWide(ex.what()).c_str());
	}

	RestoreGuiParams(config.getGpuList(), suggestedRamSize);

	m_Tray.CreateIcon(m_hWnd, m_Icon, SWM_TRAYINTERACTION);
	m_Tray.SetTooltip(wszAppName, config.getDriveLetter());

	m_RamDrive.SetStateChangeCallback([&]() {
		m_UpdateState = true;
		InvalidateRect(m_hWnd, NULL, FALSE);
	});
	m_UpdateState = true;
}

void GpuRamGui::RestoreGuiParams(DWORD gpu, DWORD suggestedRamSize)
{
	config.setGpuList(gpu);
	m_Tray.SetTooltip(wszAppName, config.getDriveLetter());

	ComboBox_SetCurSel(m_CtlGpuList, gpu);
	ComboBox_SetCurSel(m_CtlDriveLetter, config.getDriveLetter());
	ComboBox_SetCurSel(m_CtlDriveType, config.getDriveType());
	ComboBox_SetCurSel(m_CtlDriveRemovable, config.getDriveRemovable());
	ComboBox_SetCurSel(m_CtlDriveFormat, config.getDriveFormat());
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
}

void GpuRamGui::SaveGuiParams(DWORD gpu)
{
	config.setGpuList(gpu);
	config.setDriveLetter(ComboBox_GetCurSel(m_CtlDriveLetter));
	config.setDriveType(ComboBox_GetCurSel(m_CtlDriveType));
	config.setDriveRemovable(ComboBox_GetCurSel(m_CtlDriveRemovable));
	config.setDriveFormat(ComboBox_GetCurSel(m_CtlDriveFormat));
	config.setMemSize(ComboBox_GetCurSel(m_CtlDriveFormat));
	config.setTempFolder(Button_GetCheck(m_CtlTempFolder));
	config.setStartOnWindows(Button_GetCheck(m_CtlStartOnWindows));

	wchar_t szTemp[64] = { 0 };
	Edit_GetText(m_CtlMemSize, szTemp, sizeof(szTemp) / sizeof(wchar_t));
	config.setMemSize((size_t)_wtoi64(szTemp));

	Edit_GetText(m_CtlDriveLabel, szTemp, sizeof(szTemp) / sizeof(wchar_t));
	config.setDriveLabel(szTemp);
}

bool GpuRamGui::CheckDriveIsMounted(char letter, wchar_t* type)
{
	wchar_t szTemp[64];
	_snwprintf_s(szTemp, sizeof(szTemp), L"%c:\\", letter);
	UINT res = GetDriveType(szTemp);
	if (type != NULL)
	{
		switch (res)
		{
		case 0:
			_tcsncpy(type, L"Indetermined", sizeof(L"Indetermined"));
			break;
		case 1:
			//_tcsncpy(type, L"Not available", sizeof(L"Not available"));
			_tcsncpy(type, L"", sizeof(L""));
			break;
		case 2:
			_tcsncpy(type, L"Removable", sizeof(L"Removable"));
			break;
		case 3:
			_tcsncpy(type, L"HardDisk", sizeof(L"HardDisk"));
			break;
		case 4:
			_tcsncpy(type, L"Network", sizeof(L"Network"));
			break;
		case 5:
			_tcsncpy(type, L"CD-ROM", sizeof(L"CD-ROM"));
			break;
		case 6:
			_tcsncpy(type, L"RamDisk", sizeof(L"RamDisk"));
			break;
		default: 
			_tcsncpy(type, L"Indetermined", sizeof(L"Indetermined"));
		}
	}

	return res > 1;
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

		SaveGuiParams(ComboBox_GetCurSel(m_CtlGpuList));

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

		if (memSize >= vGpu[n].memsize) {
			MessageBox(m_hWnd, L"The memory size you specified is too large", L"Invalid memory size", MB_OK);
			return;
		}

		ComboBox_GetText(m_CtlDriveLetter, szTemp, sizeof(szTemp) / sizeof(wchar_t));
		wchar_t* mountPointParam = szTemp;

		if (CheckDriveIsMounted(mountPointParam[0], NULL))
		{
			MessageBox(m_hWnd, L"It is not possible to mount the unit, it is already in use", wszAppName, MB_OK);
			return;
		}

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
		EnableWindow(m_CtlDriveLabel, FALSE);
		EnableWindow(m_CtlDriveFormat, FALSE);
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
		EnableWindow(m_CtlDriveLabel, TRUE);
		EnableWindow(m_CtlDriveFormat, TRUE);
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
		case WM_CLOSE:
			if (_this->m_RamDrive.IsMounted()) {
				if (MessageBox(hWnd, L"The drive is mounted, do you really want to exit?", _this->wszAppName, MB_OKCANCEL) == IDOK)
				{
					DestroyWindow(hWnd);
				}
			}
			else {
					DestroyWindow(hWnd);
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

		case WM_COMMAND:
		{
			if (_this) {
				if ((HANDLE)lParam == _this->m_CtlGpuList ||
					(HANDLE)lParam == _this->m_CtlDriveLetter ||
					(HANDLE)lParam == _this->m_CtlDriveFormat ||
					(HANDLE)lParam == _this->m_CtlDriveRemovable ||
					(HANDLE)lParam == _this->m_CtlDriveType) {
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						_this->SaveGuiParams(_this->config.getGpuList());
						_this->m_Tray.SetTooltip(_this->wszAppName, _this->config.getDriveLetter());
					}
				}

				if ((HANDLE)lParam == _this->m_CtlMountBtn) {
					EnableWindow(_this->m_CtlMountBtn, FALSE);
					_this->OnMountClicked();
					EnableWindow(_this->m_CtlMountBtn, TRUE);
				}
				else if ((HANDLE)lParam == _this->m_CtlGpuList) {
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						LRESULT itemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

						TGPUDevice it = _this->m_RamDrive.GetGpuDevices().at(itemIndex);
#if GPU_API == GPU_API_HOSTMEM
						int suggestedRamSize = 256;
#else
						int suggestedRamSize = (int)((it.memsize / 1024 / 1024) - 1024);
#endif
						_this->RestoreGuiParams(itemIndex, suggestedRamSize);
					}
				}
				else if ((HANDLE)lParam == _this->m_CtlTempFolder) {
					BOOL checked = Button_GetCheck(_this->m_CtlTempFolder);
					Button_SetCheck(_this->m_CtlTempFolder, !checked);
					_this->SaveGuiParams(_this->config.getGpuList());

					if (!checked) {
						_this->config.SaveOriginalTempEnvironment();
					}
				}
				else if ((HANDLE)lParam == _this->m_CtlStartOnWindows) {
					BOOL checked = Button_GetCheck(_this->m_CtlStartOnWindows);
					Button_SetCheck(_this->m_CtlStartOnWindows, !checked);
					_this->SaveGuiParams(_this->config.getGpuList());

					TaskManager taskManager;

					wchar_t taskJobName[MAX_PATH] = {};
					_snwprintf_s(taskJobName, sizeof(taskJobName), L"%s_%d", _this->wszTaskJobName, _this->config.getGpuList());
					if (!checked) {
						wchar_t nPath[MAX_PATH] = {};
						wchar_t nArguments[MAX_PATH] = {};

						GetModuleFileName(NULL, nPath, MAX_PATH);
						_snwprintf_s(nArguments, sizeof(nArguments), L"--device %d --hide", _this->config.getGpuList());

						taskManager.CreateTaskJob(taskJobName, nPath, nArguments);
					}
					else {
						taskManager.DeleteTaskJob(taskJobName);
					}
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
