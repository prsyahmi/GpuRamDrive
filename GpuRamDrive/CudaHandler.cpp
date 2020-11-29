#include "stdafx.h"
#include "CudaHandler.h"

#if GPU_API == GPU_API_CUDA
#pragma comment(lib, "cuda.lib")

CudaHandler* CudaHandler::pinstance_{ nullptr };
std::mutex CudaHandler::mutex_;

CudaHandler::CudaHandler()
	: contexts()
	, contextsMux()
	, debugTools(L"GpuRamDrive")
{
	debugTools.deb(L"Cuda initialize");
	cuInit(0);
}

CudaHandler::~CudaHandler()
{
	for (auto const& x : contexts)
	{
		debugTools.deb(L"Cuda context %u destroy", x.first);
		if (x.second) cuCtxDestroy(x.second);
	}
}

CudaHandler* CudaHandler::GetInstance()
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (pinstance_ == nullptr)
	{
		pinstance_ = new CudaHandler();
	}
	return pinstance_;
}


CUcontext CudaHandler::getContext(cl_device_id clDeviceId)
{
	CUdevice m_cuDev = (CUdevice)(UINT_PTR)(clDeviceId);
	if (contexts.find(m_cuDev) == contexts.end())
	{
		debugTools.deb(L"Generating new context for cuda device: '%d'", m_cuDev);
		CUcontext m_cuCtx;
		cuDevicePrimaryCtxRetain(&m_cuCtx, m_cuDev);
		contexts[m_cuDev] = m_cuCtx;
		contextsMux[m_cuDev] = 1;
	}
	else
	{
		contextsMux[m_cuDev]++;
	}
	return contexts[m_cuDev];
}

void CudaHandler::removeContext(cl_device_id clDeviceId)
{
	CUdevice m_cuDev = (CUdevice)(UINT_PTR)(clDeviceId);
	contextsMux[m_cuDev]--;
	if (contextsMux[m_cuDev] == 0)
	{
		debugTools.deb(L"Destroying the context for cuda device: '%d'", m_cuDev);
		//cuCtxDestroy(contexts[m_cuDev]);
		cuDevicePrimaryCtxRelease(m_cuDev);
		contexts[m_cuDev] = nullptr;
		contexts.erase(m_cuDev);
	}
}
#endif
