#pragma once
#define GPU_API_CUDA    1

#if GPU_API == GPU_API_CUDA
#include <map>
#include <mutex>
#include <cuda.h>
#include "DebugTools.h"

class CudaHandler
{
private:
	static CudaHandler* pinstance_;
	static std::mutex mutex_;
	std::map<CUdevice, CUcontext> contexts;
	std::map<CUdevice, int> contextsMux;
	DebugTools debugTools;

protected:
	CudaHandler();
	~CudaHandler();

public:
	CudaHandler(CudaHandler& other) = delete;
	void operator=(const CudaHandler&) = delete;
	static CudaHandler* GetInstance();
	CUcontext getContext(cl_device_id clDeviceId);
	void removeContext(cl_device_id clDeviceId);
};
#endif
