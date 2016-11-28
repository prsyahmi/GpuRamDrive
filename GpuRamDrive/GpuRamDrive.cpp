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
#include <imdisk/imdisk.h>
#include <imdisk/imdproxy.h>
#include "GpuRamDrive.h"

#define TEST_HOST_RAM 0

GPURamDrive::GPURamDrive()
	: m_MemSize(0)
	, m_Context(nullptr)
	, m_Queue(nullptr)
	, m_GpuMem(nullptr)
	, m_pBuff(nullptr)
	, m_ImdDrive(INVALID_HANDLE_VALUE)
	, m_ShmHandle(NULL)
	, m_ShmMutexSrv(NULL)
	, m_ShmReqEvent(NULL)
	, m_ShmRespEvent(NULL)
	, m_ShmView(nullptr)
{

}

GPURamDrive::~GPURamDrive()
{
	Close();
}

void GPURamDrive::RefreshGPUInfo()
{
	cl_int clRet;
	cl_platform_id platforms[8];
	cl_uint numPlatforms;

	if ((clRet = clGetPlatformIDs(4, platforms, &numPlatforms)) != CL_SUCCESS) {
		throw std::runtime_error(std::string("Unable to get platform IDs: ") + std::to_string(clRet));
	}

	for (cl_uint i = 0; i < numPlatforms; i++) {
		cl_device_id devices[16];
		cl_uint numDevices;
		char szPlatformName[64] = { 0 };

		if ((clRet = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(szPlatformName), szPlatformName, nullptr)) != CL_SUCCESS) {
			continue;
		}

		if ((clRet = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR, 16, devices, &numDevices)) != CL_SUCCESS) {
			continue;
		}

		for (cl_uint j = 0; j < numDevices; j++) {
			TGPUDevice GpuDevices;
			char szDevName[64] = { 0 };

			if ((clRet = clGetDeviceInfo(devices[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &GpuDevices.memsize, nullptr)) != CL_SUCCESS) {
				continue;
			}

			if ((clRet = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(szDevName), szDevName, nullptr)) != CL_SUCCESS) {
				continue;
			}

			GpuDevices.platform_id = platforms[i];
			GpuDevices.device_id = devices[j];
			GpuDevices.name = szPlatformName + std::string(" - ") + szDevName;
			m_Devices.push_back(GpuDevices);
		}
	}
}

const std::vector<TGPUDevice>& GPURamDrive::GetGpuDevices()
{
	return m_Devices;
}

void GPURamDrive::CreateRamDevice(cl_platform_id PlatformId, cl_device_id DeviceId, const std::wstring& ServiceName, safeio_size_t MemSize, const wchar_t* MountPoint)
{
	m_PlatformId = PlatformId;
	m_DeviceId = DeviceId;
	m_MemSize = MemSize;
	m_ServiceName = ServiceName;

	std::exception state_ex;
	std::atomic<int> state = 0;

	m_GpuThread = std::thread([&]() {
		try
		{
			GpuAllocateRam();
			ImdiskSetupComm(ServiceName);
			state = 1;
			ImdiskHandleComm();
		}
		catch (const std::exception& ex)
		{
			Close();
			state_ex = ex;
			state = 2;
		}
	});

	while (state == 0) {
		Sleep(1);
	}

	if (state == 2) {
		if (m_GpuThread.joinable()) m_GpuThread.join();
		throw state_ex;
	}

	ImdiskMountDevice(MountPoint);
	if (m_StateChangeCallback) m_StateChangeCallback();
}

void GPURamDrive::ImdiskMountDevice(const wchar_t* MountPoint)
{
	DISK_GEOMETRY dskGeom = { 0 };

	ImDiskSetAPIFlags(IMDISK_API_FORCE_DISMOUNT);

	m_MountPoint = MountPoint;
	if (!ImDiskCreateDevice(NULL, &dskGeom, nullptr, IMDISK_TYPE_PROXY | IMDISK_PROXY_TYPE_SHM | IMDISK_DEVICE_TYPE_HD, m_ServiceName.c_str(), FALSE, (LPWSTR)MountPoint)) {
		throw std::runtime_error("Unable to create and mount ImDisk drive");
	}
}

void GPURamDrive::ImdiskUnmountDevice()
{
	if (m_MountPoint.length() == 0) return;
	
	ImDiskRemoveDevice(NULL, 0, m_MountPoint.c_str());
	m_MountPoint.clear();
}

void GPURamDrive::Close()
{
	ImdiskUnmountDevice();

	if (m_GpuThread.get_id() != std::this_thread::get_id()) {
		if (m_GpuThread.joinable()) m_GpuThread.join();
	}

	if (m_ShmView) UnmapViewOfFile(m_ShmView);
	if (m_ShmHandle) CloseHandle(m_ShmHandle);
	if (m_ShmMutexSrv) CloseHandle(m_ShmMutexSrv);
	if (m_ShmReqEvent) CloseHandle(m_ShmReqEvent);
	if (m_ShmRespEvent) CloseHandle(m_ShmRespEvent);

	if (m_pBuff) delete[] m_pBuff;
	if (m_GpuMem) clReleaseMemObject(m_GpuMem);
	if (m_Queue) clReleaseCommandQueue(m_Queue);
	if (m_Context) clReleaseContext(m_Context);

	m_ShmView = nullptr;
	m_ShmHandle = NULL;
	m_ShmMutexSrv = NULL;
	m_ShmReqEvent = NULL;
	m_ShmRespEvent = NULL;

	m_pBuff = nullptr;
	m_GpuMem = nullptr;
	m_Queue = nullptr;
	m_Context = nullptr;
	m_MemSize = 0;

	if (m_StateChangeCallback) m_StateChangeCallback();
}

bool GPURamDrive::IsMounted()
{
	return m_MountPoint.size() != 0 && m_ShmView != nullptr;
}

void GPURamDrive::SetStateChangeCallback(const std::function<void()> callback)
{
	m_StateChangeCallback = callback;
}

void GPURamDrive::GpuAllocateRam()
{
#if TEST_HOST_RAM
	m_pBuff = new char[m_MemSize];
#else
	cl_int clRet;

	m_Context = clCreateContext(nullptr, 1, &m_DeviceId, nullptr, nullptr, &clRet);
	if (m_Context == nullptr) {
		throw std::runtime_error("Unable to create context: " + std::to_string(clRet));
	}

	m_Queue = clCreateCommandQueue(m_Context, m_DeviceId, 0, &clRet);
	if (m_Queue == nullptr) {
		throw std::runtime_error("Unable to create command queue: " + std::to_string(clRet));
	}

	m_GpuMem = clCreateBuffer(m_Context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, m_MemSize, nullptr, &clRet);
	if (m_GpuMem == nullptr) {
		throw std::runtime_error("Unable to create memory buffer: " + std::to_string(clRet));
	}
#endif
}

safeio_ssize_t GPURamDrive::GpuWrite(void *buf, safeio_size_t size, off_t_64 offset)
{
#if TEST_HOST_RAM
	memcpy(m_pBuff + offset, buf, size);
	return size;
#else
	if (clEnqueueWriteBuffer(m_Queue, m_GpuMem, CL_TRUE, (size_t)offset, (size_t)size, buf, 0, nullptr, nullptr) != CL_SUCCESS) {
		return 0;
	}

	return size;
#endif
}

safeio_ssize_t GPURamDrive::GpuRead(void *buf, safeio_size_t size, off_t_64 offset)
{
#if TEST_HOST_RAM
	memcpy(buf, m_pBuff + offset, size);
	return size;
#else
	if (clEnqueueReadBuffer(m_Queue, m_GpuMem, CL_TRUE, (size_t)offset, (size_t)size, buf, 0, nullptr, nullptr) != CL_SUCCESS) {
		return 0;
	}

	return size;
#endif
}

void GPURamDrive::ImdiskSetupComm(const std::wstring& ServiceName)
{
	MEMORY_BASIC_INFORMATION MemInfo;
	ULARGE_INTEGER MapSize;
	DWORD dwErr;
	std::wstring sTemp;
	const std::wstring sPrefix = L"Global\\";

	m_BufSize = (2 << 20);
	MapSize.QuadPart = m_BufSize + IMDPROXY_HEADER_SIZE;

	sTemp = sPrefix + ServiceName;
	m_ShmHandle = CreateFileMapping(INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE | SEC_COMMIT,
		MapSize.HighPart,
		MapSize.LowPart,
		sTemp.c_str());
	dwErr = GetLastError();
	if (m_ShmHandle == NULL) {
		throw std::runtime_error("Unable to create file mapping: " + std::to_string(dwErr));
	}

	if (dwErr == ERROR_ALREADY_EXISTS) {
		throw std::runtime_error("A service with this name is already running or is still being used by ImDisk");
	}

	m_ShmView = MapViewOfFile(m_ShmHandle, FILE_MAP_WRITE, 0, 0, 0);
	if (m_ShmView == nullptr) {
		dwErr = GetLastError();
		throw std::runtime_error("Unable to map view of shared memory: " + std::to_string(dwErr));
	}

	if (!VirtualQuery(m_ShmView, &MemInfo, sizeof(MemInfo))) {
		dwErr = GetLastError();
		throw std::runtime_error("Unable to query memory info: " + std::to_string(dwErr));
	}

	m_BufStart = (char*)m_ShmView + IMDPROXY_HEADER_SIZE;


	sTemp = sPrefix + ServiceName + L"_Server";
	m_ShmMutexSrv = CreateMutex(NULL, FALSE, sTemp.c_str());
	if (m_ShmMutexSrv == NULL) {
		dwErr = GetLastError();
		throw std::runtime_error("Unable to create mutex object: " + std::to_string(dwErr));
	}

	if (WaitForSingleObject(m_ShmMutexSrv, 0) != WAIT_OBJECT_0) {
		throw std::runtime_error("A service with this name is already running");
	}

	sTemp = sPrefix + ServiceName + L"_Request";
	m_ShmReqEvent = CreateEvent(NULL, FALSE, FALSE, sTemp.c_str());
	if (m_ShmReqEvent == NULL) {
		dwErr = GetLastError();
		throw std::runtime_error("Unable to create request event object: " + std::to_string(dwErr));
	}

	sTemp = sPrefix + ServiceName + L"_Response";
	m_ShmRespEvent = CreateEvent(NULL, FALSE, FALSE, sTemp.c_str());
	if (m_ShmRespEvent == NULL) {
		dwErr = GetLastError();
		throw std::runtime_error("Unable to create response event object: " + std::to_string(dwErr));
	}
}

void GPURamDrive::ImdiskHandleComm()
{
	PIMDPROXY_READ_REQ Req = (PIMDPROXY_READ_REQ)m_ShmView;
	PIMDPROXY_READ_RESP Resp = (PIMDPROXY_READ_RESP)m_ShmView;

	for (;;)
	{
		if (WaitForSingleObject(m_ShmReqEvent, INFINITE) != WAIT_OBJECT_0) {
			return;
		}

		switch (Req->request_code)
		{
			case IMDPROXY_REQ_INFO:
			{
				PIMDPROXY_INFO_RESP resp = (PIMDPROXY_INFO_RESP)m_ShmView;
				resp->file_size = m_MemSize;
				resp->req_alignment = 1;
				resp->flags = 0;
				break;
			}

			case IMDPROXY_REQ_READ:
			{
				Resp->errorno = 0;
				Resp->length = GpuRead(m_BufStart, (safeio_size_t)(Req->length < m_BufSize ? Req->length : m_BufSize), Req->offset);

				break;
			}

			case IMDPROXY_REQ_WRITE:
			{
				Resp->errorno = 0;
				Resp->length = GpuWrite(m_BufStart, (safeio_size_t)(Req->length < m_BufSize ? Req->length : m_BufSize), Req->offset);

				break;
			}

			case IMDPROXY_REQ_CLOSE:
				return;

			default:
				Req->request_code = ENODEV;
		}

		if (!SetEvent(m_ShmRespEvent)) {
			return;
		}
	}
}
