#pragma once

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

	cl_context m_Context;
	cl_command_queue m_Queue;
	cl_mem m_GpuMem;

	HANDLE m_ShmHandle;
	HANDLE m_ShmMutexSrv;
	HANDLE m_ShmReqEvent;
	HANDLE m_ShmRespEvent;
	void* m_ShmView;

	void* m_BufStart;

public:
	GPURamDrive();
	~GPURamDrive();

	void RefreshGPUInfo();
	const std::vector<TGPUDevice>& GetGpuDevices();

	void CreateRamDevice(cl_platform_id PlatformId, cl_device_id DeviceId, const std::wstring& ServiceName, safeio_size_t MemSize);
	void CreateDosDevice(char DriveLetter);
	void Close();

private:
	void GpuAllocateRam();
	safeio_ssize_t GpuWrite(void *buf, safeio_size_t size, off_t_64 offset);
	safeio_ssize_t GpuRead(void *buf, safeio_size_t size, off_t_64 offset);

	void ImdiskSetupComm(const std::wstring& ServiceName);
	void ImdiskHandleComm();
};
