#pragma once
#include "GpuRamTrayIcon.h"
#include "Config.h"


class GpuRamGui
{
private:
	GPURamDrive m_RamDrive;
	HWND m_hWnd;
	HINSTANCE m_Instance;
	GpuRamTrayIcon m_Tray;

	HICON m_Icon;
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
	HWND m_CtlTempFolder;
	HWND m_CtlStartOnWindows;
	bool m_UpdateState;

	LPCWSTR wszAppName;
	LPCWSTR wszTaskJobName;

	Config config;
	DiskUtil diskUtil;

public:
	GpuRamGui();
	~GpuRamGui();

	bool Create(HINSTANCE hInst, const std::wstring& title, int nCmdShow);
	int Loop();
	void Mount(DWORD gpu);
	void RestoreWindow();

private:
	void OnCreate();
	void OnDestroy();
	void OnEndSession();
	void OnResize(WORD width, WORD height, bool minimized);
	void RestoreGuiParams(DWORD gpu, DWORD suggestedRamSize);
	void SaveGuiParams(DWORD gpu);
	void OnMountClicked();
	void OnTrayInteraction(LPARAM lParam);
	void UpdateState();

	ATOM MyRegisterClass();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

