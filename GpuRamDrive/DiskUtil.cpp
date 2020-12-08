#include <stdio.h>
#include <commdlg.h>
#include "stdafx.h"
#include "DiskUtil.h"

#define dump_buffersize_megs 16
#define dump_buffersize (dump_buffersize_megs * 1024 * 1024)
#define dump_workingsetsize ((dump_buffersize_megs + 1) * 1024 * 1024)

DiskUtil::DiskUtil()
    : buffer(NULL)
    ,hinput(NULL)
    ,houtput(NULL)
{
}

DiskUtil::~DiskUtil()
{
}

BOOL DiskUtil::checkDriveIsMounted(char letter, wchar_t* type)
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

BOOL DiskUtil::fileExists(LPCTSTR szPath)
{
    DWORD dwAttrib = GetFileAttributes(szPath);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

std::wstring DiskUtil::chooserFile(const wchar_t* title, const wchar_t* filter)
{
    wchar_t szFile[MAX_PATH];
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.lpstrTitle = title;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    if (!GetOpenFileName(&ofn)) {
        return L"";
    }

    return ofn.lpstrFile;

    /*
    WCHAR szPath[MAX_PATH + 1] = { 0 };
    DWORD retval = GetFullPathNameW(ofn.lpstrFile, MAX_PATH, szPath, NULL);
    if ((retval == 0) || (retval > MAX_PATH))
        return L"";

    std::wstring wstrPath(szPath, retval);
    std::wcout << wstrPath << endl;
    return wstrPath;
    */
}

void DiskUtil::createDriveIcon(char letter)
{
    wchar_t keyName[128 + 1] = { 0 };
    _stprintf(keyName, _T("SOFTWARE\\Classes\\Applications\\Explorer.exe\\Drives\\%c\\DefaultIcon"), letter);
    LPTSTR path = new TCHAR[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);

    wchar_t pszValue[MAX_PATH + 10] = { 0 };
    _stprintf(pszValue, _T("%s,0"), path);

    obKey.SetKeyValue(keyName, L"", pszValue, (DWORD)wcslen(pszValue) * sizeof(wchar_t), true, false, HKEY_CURRENT_USER);
}

void DiskUtil::removeDriveIcon(char letter)
{
    LPTSTR keyName = new TCHAR[128 + 1];
    _stprintf(keyName, _T("SOFTWARE\\Classes\\Applications\\Explorer.exe\\Drives\\%c"), letter);
    obKey.QuickDeleteKey(keyName, HKEY_CURRENT_USER);
}

void DiskUtil::throwError(const char* text)
{
    DWORD err = GetLastError();

    freeResources();
    char message[1024] = {};
    sprintf(message, text, err);
    throw std::runtime_error(message);
}

void DiskUtil::freeResources()
{
    if (buffer != NULL)
    {
        VirtualUnlock(buffer, dump_buffersize);
        VirtualFree(buffer, 0, MEM_RELEASE);
    }
    if (hinput != NULL && hinput != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hinput);
    }
    if (houtput != NULL && houtput != INVALID_HANDLE_VALUE)
    {
        CloseHandle(houtput);
    }
    buffer = NULL;
    hinput = NULL;
    houtput = NULL;
}

DWORD DiskUtil::save(const wchar_t* source_device_name, const wchar_t* filename)
{
    DWORD err;
    DWORD bytes_to_transfer, byte_count;
    GET_LENGTH_INFORMATION source_disklength;
    DISK_GEOMETRY source_diskgeometry;
    LARGE_INTEGER offset;
    OVERLAPPED overlapped;

    if (!SetProcessWorkingSetSize(GetCurrentProcess(), dump_workingsetsize, dump_workingsetsize))
    {
        throwError("Error %u trying to expand working set");
    }

    buffer = VirtualAlloc(NULL, dump_buffersize, MEM_COMMIT, PAGE_READWRITE);

    if (buffer == NULL)
    {
        throwError("Error %u trying to allocate buffer");
    }

    if (!VirtualLock(buffer, dump_buffersize))
    {
        throwError("Error %u trying to lock buffer");
    }

    hinput = CreateFile(source_device_name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);

    if (hinput == INVALID_HANDLE_VALUE) {
        throwError("Error %u opening input device");
    }

    if (!DeviceIoControl(hinput, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &byte_count, NULL))
    {
        throwError("Error %u locking input volume");
    }

    if (!DeviceIoControl(hinput, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &source_diskgeometry, sizeof(source_diskgeometry), &byte_count, NULL))
    {
        throwError("Error %u getting device geometry");
    }

    switch (source_diskgeometry.MediaType)
    {
    case Unknown:
    case RemovableMedia:
    case FixedMedia:

        if (!DeviceIoControl(hinput, FSCTL_ALLOW_EXTENDED_DASD_IO, NULL, 0, &source_disklength, sizeof(source_disklength), &byte_count, NULL))
        {
            throwError("Error %u getting input device dasd");
        }
        if (!DeviceIoControl(hinput, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &source_disklength, sizeof(source_disklength), &byte_count, NULL))
        {
            throwError("Error %u getting input device length");
        }

        fprintf(stderr, "\nInput disk has %I64i bytes.\n\n", source_disklength.Length.QuadPart);
        break;

    default:

        source_disklength.Length.QuadPart =
            source_diskgeometry.Cylinders.QuadPart *
            source_diskgeometry.TracksPerCylinder *
            source_diskgeometry.SectorsPerTrack *
            source_diskgeometry.BytesPerSector;

        fprintf(stderr,
            "\n"
            "Input device appears to be a floppy disk.  WARNING: if this is not a\n"
            "floppy disk the calculated size will probably be incorrect, resulting\n"
            "in an incomplete copy.\n"
            "\n"
            "Input disk has %I64i bytes.\n"
            "\n",
            source_disklength.Length.QuadPart);

        break;
    }

    houtput = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);

    if (houtput == INVALID_HANDLE_VALUE)
    {
        throwError("Error %u creating output file");
    }

    offset.QuadPart = 0;
    overlapped.hEvent = 0;

    for (;;)
    {
        overlapped.Offset = offset.LowPart;
        overlapped.OffsetHigh = offset.HighPart;

        if (source_disklength.Length.QuadPart - offset.QuadPart < dump_buffersize)
        {
            bytes_to_transfer = (DWORD)(source_disklength.Length.QuadPart - offset.QuadPart);
            if (bytes_to_transfer == 0) break;
        }
        else
        {
            bytes_to_transfer = dump_buffersize;
        }

        if (!ReadFile(hinput, buffer, bytes_to_transfer, NULL, &overlapped))
        {
            throwError("Error %u initiating read from input disk");
        }

        if (!GetOverlappedResult(hinput, &overlapped, &byte_count, TRUE))
        {
            throwError("Error %u reading from input disk");
        }

        if (byte_count != bytes_to_transfer)
        {
            err = GetLastError();
            printf("Internal error - partial read.  Last error code %u.\n", err);
            printf("bytes_to_transfer = %u; byte_count = %u.\n", bytes_to_transfer, byte_count);
            if (byte_count == 0)
            {
                freeResources();
                return ERROR_INVALID_FUNCTION;
            }
            bytes_to_transfer = byte_count;
        }

        if (!WriteFile(houtput, buffer, bytes_to_transfer, NULL, &overlapped))
        {
            err = GetLastError();
            if (err != ERROR_IO_PENDING)
            {
                throwError("Error %u initiating write to output file");
            }
        }

        if (!GetOverlappedResult(houtput, &overlapped, &byte_count, TRUE))
        {
            throwError("Error %u writing to output file");
        }

        if (byte_count != bytes_to_transfer)
        {
            printf("Internal error - partial write.\n");
            printf("bytes_to_transfer = %u; byte_count = %u.\n", bytes_to_transfer, byte_count);
            freeResources();
            return ERROR_INVALID_FUNCTION;
        }

        offset.QuadPart += bytes_to_transfer;
    }

    overlapped.Offset = offset.LowPart;
    overlapped.OffsetHigh = offset.HighPart;

    if (!ReadFile(hinput, buffer, source_diskgeometry.BytesPerSector, NULL, &overlapped))
    {
        err = GetLastError();
        if (err == ERROR_HANDLE_EOF)
        {
            freeResources();
            return 0;
        }
        throwError("Error %u initiating read from input disk past end of file");
    }

    if (!GetOverlappedResult(hinput, &overlapped, &byte_count, TRUE))
    {
        err = GetLastError();
        if (err == ERROR_HANDLE_EOF)
        {
            freeResources();
            return 0;
        }
        throwError("Error %u reading from input disk past end of file");
    }

    if (byte_count == 0)
    {
        freeResources();
        return 0;
    }

    printf("WARNING: the expected amount of data was successfully copied,\n"
        "but end of file not detected on input disk.  The copy might\n"
        "not be complete.");

    return ERROR_MORE_DATA;
}

DWORD DiskUtil::restore(const wchar_t* filename, const wchar_t* target_device_name) {

    DWORD err;
    WIN32_FILE_ATTRIBUTE_DATA fad;
    DWORD bytes_to_transfer, byte_count;
    LARGE_INTEGER filelength;
    GET_LENGTH_INFORMATION target_disklength;
    DISK_GEOMETRY target_diskgeometry;
    LARGE_INTEGER transfer_length;
    LARGE_INTEGER offset;
    OVERLAPPED overlapped;

    if (!SetProcessWorkingSetSize(GetCurrentProcess(), dump_workingsetsize, dump_workingsetsize))
    {
        throwError("Error %u trying to expand working set");
    }

    buffer = VirtualAlloc(NULL, dump_buffersize, MEM_COMMIT, PAGE_READWRITE);

    if (buffer == NULL)
    {
        throwError("Error %u trying to allocate buffer");
    }

    if (!VirtualLock(buffer, dump_buffersize))
    {
        throwError("Error %u trying to lock buffer");
    }

    if (!GetFileAttributesEx(filename, GetFileExInfoStandard, &fad))
    {
        throwError("Error %u reading input file attributes");
    }

    filelength.HighPart = fad.nFileSizeHigh;
    filelength.LowPart = fad.nFileSizeLow;

    hinput = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);

    if (hinput == INVALID_HANDLE_VALUE)
    {
        throwError("Error %u opening input file");
    }

    houtput = CreateFile(target_device_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);

    if (houtput == INVALID_HANDLE_VALUE) {
        throwError("Error %u opening output device");
    }

    if (!DeviceIoControl(houtput, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &byte_count, NULL))
    {
        throwError("Error %u locking volume");
    }

    if (!DeviceIoControl(houtput, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &target_diskgeometry, sizeof(target_diskgeometry), &byte_count, NULL))
    {
        throwError("Error %u getting output device geometry");
    }

    switch (target_diskgeometry.MediaType)
    {
    case Unknown:
    case RemovableMedia:
    case FixedMedia:

        if (!DeviceIoControl(houtput, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &target_disklength, sizeof(target_disklength), &byte_count, NULL))
        {
            throwError("Error %u getting output device length");
        }

        break;

    default:

        target_disklength.Length.QuadPart =
            target_diskgeometry.Cylinders.QuadPart *
            target_diskgeometry.TracksPerCylinder *
            target_diskgeometry.SectorsPerTrack *
            target_diskgeometry.BytesPerSector;

        fprintf(stderr,
            "\n"
            "Output device appears to be a floppy disk.  WARNING: if this is not a\n"
            "floppy disk the calculated output device size is probably incorrect,\n"
            "which might result in an incomplete copy.\n"
            "\n"
            "Output disk has %I64i bytes.\n"
            "\n",
            target_disklength.Length.QuadPart);

        break;
    }

    if (filelength.QuadPart == target_disklength.Length.QuadPart)
    {
        transfer_length.QuadPart = filelength.QuadPart;
    }
    else if (filelength.QuadPart < target_disklength.Length.QuadPart)
    {
        fprintf(stderr, "Image is smaller than target.  Part of the target will not be written to.\n\n");
        transfer_length.QuadPart = filelength.QuadPart;
    }
    else
    {
        fprintf(stderr, "Image is larger than target.  Part of the image will not be copied.\n\n");
        transfer_length.QuadPart = target_disklength.Length.QuadPart;
    }

    offset.QuadPart = 0;
    overlapped.hEvent = 0;

    for (;;)
    {
        overlapped.Offset = offset.LowPart;
        overlapped.OffsetHigh = offset.HighPart;

        if (transfer_length.QuadPart - offset.QuadPart < dump_buffersize)
        {
            bytes_to_transfer = (DWORD)(transfer_length.QuadPart - offset.QuadPart);
            if (bytes_to_transfer == 0) break;
        }
        else
        {
            bytes_to_transfer = dump_buffersize;
        }

        if (!ReadFile(hinput, buffer, bytes_to_transfer, NULL, &overlapped))
        {
            err = GetLastError();
            if (err != ERROR_IO_PENDING)
            {
                throwError("Error %u initiating read from input file");
            }
        }

        if (!GetOverlappedResult(hinput, &overlapped, &byte_count, TRUE))
        {
            throwError("Error %u reading from input file");
        }

        if (byte_count != bytes_to_transfer)
        {
            err = GetLastError();
            printf("Internal error - partial read.  Last error code %u.\n", err);
            printf("bytes_to_transfer = %u; byte_count = %u.\n", bytes_to_transfer, byte_count);
            if (byte_count == 0)
            {
                freeResources();
                return ERROR_INVALID_FUNCTION;
            }
            bytes_to_transfer = byte_count;
        }

        if (!WriteFile(houtput, buffer, bytes_to_transfer, NULL, &overlapped))
        {
            err = GetLastError();
            if (err != ERROR_IO_PENDING)
            {
                throwError("Error %u initiating write to output disk");
            }
        }

        if (!GetOverlappedResult(houtput, &overlapped, &byte_count, TRUE))
        {
            throwError("Error %u writing to output disk");
        }

        if (byte_count != bytes_to_transfer)
        {
            printf("Internal error - partial write.\n");
            printf("bytes_to_transfer = %u; byte_count = %u.\n", bytes_to_transfer, byte_count);
            freeResources();
            return ERROR_INVALID_FUNCTION;
        }

        offset.QuadPart += bytes_to_transfer;
    }

    freeResources();
    return 0;
}