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

const wchar_t GPU_HELP_STRING[] = L"Usage:\n"
"  GpuRamDrive.exe --device 0 --hide\n"
"\n"
"Options:\n"
"  --device <Device Id>     Set the GPU device\n"
"  --hide                   Hide GUI to tray\n"
"  --help                   Show this help\n";

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	GpuRamGui gui;

	LPWSTR *szArglist;
	int nArgs;
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);

	size_t gpuDevice = -1;
	bool gpuMount = false;
	bool helpRequest = false;

	if (szArglist) {
		for (int i = 0; i < nArgs; i++) {
			if (_wcsicmp(szArglist[i], L"--device") == 0 && i + 1 < nArgs)
			{
				gpuDevice = _wtoi64(szArglist[i + 1]);
			}
			else if (_wcsicmp(szArglist[i], L"--hide") == 0)
			{
				nCmdShow = SW_HIDE;
			}
			else if (_wcsicmp(szArglist[i], L"--help") == 0 ||
				_wcsicmp(szArglist[i], L"-h") == 0 ||
				_wcsicmp(szArglist[i], L"/h") == 0 ||
				_wcsicmp(szArglist[i], L"/?") == 0)
			{
				helpRequest = true;
			}
		}

		if (helpRequest) {
			if (GetConsoleWindow() == NULL) {
				MessageBoxW(NULL, GPU_HELP_STRING, L"GpuRamDrive help", MB_OK);
			}
			wprintf(GPU_HELP_STRING);
			return 0;
		}

		gpuMount = gpuDevice >= 0 && gpuDevice < 10;
	}

	if (!gui.Create(hInstance, L"GPU Ram Drive", nCmdShow)) {
		return -1;
	}

	if (gpuMount) {
		try
		{
			gui.Mount(gpuDevice);
		}
		catch (const std::exception& ex)
		{
			gui.RestoreWindow();
			if (GetConsoleWindow() == NULL) {
				MessageBoxA(NULL, ex.what(), "GpuRamDrive error", MB_OK);
			}
			fprintf(stderr, "GpuRamDrive exception: %s\n", ex.what());
		}
	}

	return gui.Loop();
}