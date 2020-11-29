#include "stdafx.h"
#include "DebugTools.h"
#include "CudaHandler.h"

DebugTools::DebugTools(LPCTSTR pszFileName)
{
    this->pszFileName = new TCHAR[_tcslen(pszFileName) + 10];
    _tcscpy(this->pszFileName, pszFileName);
#if GPU_API == GPU_API_CUDA
    _tcscat(this->pszFileName, L"_CUDA");
#endif
    _tcscat(this->pszFileName, L".log");
}

DebugTools::~DebugTools() {}

void DebugTools::deb(wchar_t* msg, ...)
{
#if _DEBUG
    va_list ap;
	wchar_t string[2048] = {};
	wchar_t stringout[4096] = {};

	va_start(ap, msg);
	vswprintf(string, wcslen(string) - 1, msg, ap);
	va_end(ap);

	swprintf(stringout, wcslen(stringout) - 1,  L"<%X> %s\n", GetCurrentThreadId(), string);
	OutputDebugString(stringout);
    WriteToFile(this->pszFileName, stringout);
#endif
}

wchar_t* DebugTools::fmterr(DWORD err)
{
	LPVOID lpMsgBuf = NULL;
	static wchar_t szInternal[255] = { 0 };

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	lstrcpy(szInternal, (wchar_t*)lpMsgBuf);
	LocalFree(lpMsgBuf);
	return szInternal;
}

void DebugTools::WriteToFile(LPCTSTR filename, wchar_t* data)
{
    HANDLE hFile;
    DWORD dwBytesToWrite = wcslen(data) * sizeof(wchar_t);
    DWORD dwBytesWritten;
    BOOL bErrorFlag = FALSE;

    hFile = CreateFile(filename,  // name of the write
        FILE_APPEND_DATA,          // open for appending
        FILE_SHARE_READ,           // share for reading only
        NULL,                      // default security
        OPEN_ALWAYS,               // open existing file or create new file 
        FILE_ATTRIBUTE_NORMAL,     // normal file
        NULL);                     // no attr. template

    if (hFile == INVALID_HANDLE_VALUE) return;

    while (dwBytesToWrite > 0)
    {
        bErrorFlag = WriteFile(
            hFile,              // open file handle
            (LPVOID)data,       // start of data to write
            dwBytesToWrite,     // number of bytes to write
            &dwBytesWritten,    // number of bytes that were written
            NULL);              // no overlapped structure

        if (!bErrorFlag) break;

        data += dwBytesWritten;
        dwBytesToWrite -= dwBytesWritten;
    }

    CloseHandle(hFile);
}