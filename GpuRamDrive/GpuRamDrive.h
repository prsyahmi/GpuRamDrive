#pragma once

#define GPU_API_HOSTMEM 0
#define GPU_API_CUDA    1
#define GPU_API_OPENCL  2

#ifndef GPU_API
#define GPU_API         GPU_API_OPENCL
#endif

#if GPU_API == GPU_API_CUDA
#include <cuda.h>
#endif

#include "Config.h"
#include "DiskUtil.h"
#include "DebugTools.h"

struct TGPUDevice
{
	cl_platform_id platform_id{};
	cl_device_id device_id{};
	cl_ulong memsize = 0;
	std::string name;
};

enum class EGpuRamDriveType
{
	HD = IMDISK_DEVICE_TYPE_HD,
	FD = IMDISK_DEVICE_TYPE_FD,
	CD = IMDISK_DEVICE_TYPE_CD,
	RAW = IMDISK_DEVICE_TYPE_RAW,
};

class GPURamDrive
{
private:
	std::vector<TGPUDevice> m_Devices;
	EGpuRamDriveType m_DriveType;
	bool m_DriveRemovable;

	cl_platform_id m_clPlatformId;
	cl_device_id m_clDeviceId;
	DWORD m_DeviceId;
	bool m_TempFolderParam;
	size_t m_MemSize;
	size_t m_BufSize;
	std::wstring m_ServiceName;
	std::wstring m_MountPoint;
	std::thread m_GpuThread;
	std::function<void()> m_StateChangeCallback;

	cl_context m_Context;
	cl_command_queue m_Queue;
	cl_mem m_GpuMem;
	char* m_pBuff;

	HANDLE m_ImdDrive;
	HANDLE m_ShmHandle;
	HANDLE m_ShmMutexSrv;
	HANDLE m_ShmReqEvent;
	HANDLE m_ShmRespEvent;
	void* m_ShmView;

	void* m_BufStart;

	Config config;
	DebugTools debugTools;

#if GPU_API == GPU_API_CUDA
	CUdeviceptr m_cuDevPtr;
	CUdevice m_cuDev;
	CUcontext m_cuCtx;
#endif

public:
	GPURamDrive();
	~GPURamDrive();

	void RefreshGPUInfo();
	const std::vector<TGPUDevice>& GetGpuDevices();

	void SetDriveType(EGpuRamDriveType type);
	void SetDriveType(const wchar_t* type);
	void SetRemovable(bool removable);
	void CreateRamDevice(cl_platform_id PlatformId, cl_device_id DeviceId, const std::wstring& ServiceName, size_t MemSize, const wchar_t* MountPoint, const std::wstring& FormatParam, const std::wstring& LabelParam, bool TempFolderParam);
	void ImdiskMountDevice(const wchar_t* MountPoint);
	void ImdiskUnmountDevice();
	bool IsMounted();
	void SetStateChangeCallback(const std::function<void()> callback);

private:
	void Close();
	void GpuAllocateRam();
	safeio_ssize_t GpuWrite(void *buf, safeio_size_t size, off_t_64 offset);
	safeio_ssize_t GpuRead(void *buf, safeio_size_t size, off_t_64 offset);

	void ImdiskSetupComm(const std::wstring& ServiceName);
	void ImdiskHandleComm();
};
