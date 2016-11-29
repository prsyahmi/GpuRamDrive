#pragma once

#define GPU_API_HOSTMEM 0
#define GPU_API_CUDA    1
#define GPU_API_OPENCL  2

#define GPU_API         GPU_API_CUDA

#if GPU_API == GPU_API_CUDA
#include <cuda.h>
#endif


struct TGPUDevice
{
	cl_platform_id platform_id;
	cl_device_id device_id;
	cl_ulong memsize;
	std::string name;
};

class GPURamDrive
{
private:
	std::vector<TGPUDevice> m_Devices;

	cl_platform_id m_PlatformId;
	cl_device_id m_DeviceId;
	safeio_size_t m_MemSize;
	safeio_size_t m_BufSize;
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

	void CreateRamDevice(cl_platform_id PlatformId, cl_device_id DeviceId, const std::wstring& ServiceName, safeio_size_t MemSize, const wchar_t* MountPoint);
	void ImdiskMountDevice(const wchar_t* MountPoint);
	void ImdiskUnmountDevice();
	void Close();
	bool IsMounted();
	void SetStateChangeCallback(const std::function<void()> callback);

private:
	void GpuAllocateRam();
	safeio_ssize_t GpuWrite(void *buf, safeio_size_t size, off_t_64 offset);
	safeio_ssize_t GpuRead(void *buf, safeio_size_t size, off_t_64 offset);

	void ImdiskSetupComm(const std::wstring& ServiceName);
	void ImdiskHandleComm();
};
