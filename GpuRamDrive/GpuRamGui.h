#pragma once
#include <map>
#include "GpuRamDrive.h"
#include "GpuRamTrayIcon.h"
#include "DiskUtil.h"
#include "Config.h"
#include "DataGridConfig.h"

class GpuRamGui
{
private:
	std::map<DWORD, GPURamDrive> m_RamDrive;
	HWND m_hWnd;
	HINSTANCE m_Instance;
	GpuRamTrayIcon m_Tray;
	bool m_AutoMount;

	HICON m_Icon;
	HICON m_IconMounted;
	HWND m_CtlGpuList;
	HWND m_CtlMountBtn;
	HWND m_CtlMemSize;
	HWND m_CtlDriveLetter;
	HWND m_CtlDriveType;
	HWND m_CtlDriveRemovable;
	HWND m_CtlDriveLabel;
	HWND m_CtlDriveFormat;
	HWND m_CtlImageFile;
	HWND m_CtlChooseFileBtn;
	HWND m_CtlReadOnly;
	HWND m_CtlTempFolder;
	HWND m_CtlStartOnWindows;
	HWND m_CtlAddBtn;
	HWND m_CtlRemoveBtn;
	bool m_UpdateState;

	LPCWSTR wszAppName;
	LPCWSTR wszTaskJobName;

	DiskUtil diskUtil;
	Config config;
	DataGridConfig dataGridConfig;

public:
	GpuRamGui();
	~GpuRamGui();

	bool Create(HINSTANCE hInst, const std::wstring& title, int nCmdShow, bool autoMount);
	int Loop();
	void AutoMount();
	void RestoreWindow();

private:
	void OnCreate();
	void OnDestroy();
	void OnEndSession();
	void OnResize(WORD width, WORD height, bool minimized);
	void ReloadDriveLetterList();
	boolean IsMounted();
	void RestoreGuiParams(DWORD gpuId, DWORD suggestedRamSize);
	void SaveGuiParams(DWORD gpuId);
	void RemoveDevice();
	void OnMountClicked();
	void OnTrayInteraction(LPARAM lParam);
	void UpdateState();

	ATOM MyRegisterClass();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

