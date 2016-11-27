// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <codecvt>
#include <atomic>
#include <thread>
#include <functional>
#include <windows.h>
#include <Windowsx.h>
#include <Commctrl.h>
#include <winternl.h>
#include <imdisk\imdisk.h>
#include <imdisk\devio_types.h>
#include <imdisk\devio.h>
#include <cl\cl.h>

#pragma comment (lib, "imdisk.lib")
#pragma comment (lib, "opencl.lib")
#pragma comment(lib, "comctl32.lib")
