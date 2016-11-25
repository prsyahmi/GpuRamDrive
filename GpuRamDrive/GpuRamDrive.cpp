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

#define TEST_HOST_RAM 0

cl_context clContext;
cl_mem clBuff;
cl_command_queue clQueue;
char* pBuff = nullptr;

safeio_ssize_t __cdecl GpuRead(
	void *handle,
	void *buf,
	safeio_size_t size,
	off_t_64 offset)
{
#if TEST_HOST_RAM
	memcpy(buf, pBuff + offset, size);
	return size;
#else
	if (clEnqueueReadBuffer(clQueue, clBuff, CL_TRUE, offset, size, buf, 0, nullptr, nullptr) != CL_SUCCESS) {
		return 0;
	}

	return size;
#endif
}

safeio_ssize_t __cdecl GpuWrite(
	void *handle,
	void *buf,
	safeio_size_t size,
	off_t_64 offset
)
{
#if TEST_HOST_RAM
	memcpy(pBuff + offset, buf, size);
	return size;
#else
	if (clEnqueueWriteBuffer(clQueue, clBuff, CL_TRUE, offset, size, buf, 0, nullptr, nullptr) != CL_SUCCESS) {
		return 0;
	}

	return size;
#endif
}

int __cdecl GpuClose(void *handle)
{
	return 0;
}

extern "C" __declspec(dllexport) void* __cdecl GpuRamDrive(
	const char *file,
	int read_only,
	dllread_proc *dllread,
	dllwrite_proc *dllwrite,
	dllclose_proc *dllclose,
	off_t_64 *size)
{
	void* err = (void*)-1;

	cl_int clRet;
	cl_platform_id platforms[4];
	cl_uint numPlatforms;
	cl_device_id devices[16];
	cl_uint numDevices;

	*dllread = GpuRead;
	*dllwrite = GpuWrite;
	*dllclose = GpuClose;
	*size = 0llu;

	off_t_64 targetSize = _atoi64(file);

	printf("Read-only: %d\n", read_only);
	printf("Size requested: %llu\n", targetSize);

#if TEST_HOST_RAM
	pBuff = (char*)VirtualAlloc(NULL, targetSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	printf("Address = 0x%p\n", pBuff);
#else

	if ((clRet = clGetPlatformIDs(4, platforms, &numPlatforms)) != CL_SUCCESS) {
		printf("Unable to get platform IDs: %u\n", clRet);
		return err;
	}

	printf("Total platform = %u: (Hardcoded using the last one)\n", numPlatforms);
	for (cl_uint i = 0; i < numPlatforms; i++) {
		char szPlatformName[64] = { 0 };
		clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(szPlatformName), szPlatformName, nullptr);

		printf("  [%u]  %s\n", i, szPlatformName);
	}

	if ((clRet = clGetDeviceIDs(platforms[numPlatforms - 1], CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR, 16, devices, &numDevices)) != CL_SUCCESS) {
		printf("Unable to get GPU devices: %u\n", clRet);
		return err;
	}
	printf("Total devices = %u: (Hardcoded using the first one)\n", numDevices);
	for (cl_uint i = 0; i < numDevices; i++) {
		char szDevName[64] = { 0 };
		clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, sizeof(szDevName), szDevName, nullptr);
		printf("  [%u]  %s\n", i, szDevName);
	}
	printf("---------------\n\n");

	clContext = clCreateContext(nullptr, numDevices, devices, nullptr, nullptr, &clRet);
	if (clContext == nullptr) {
		printf("Unable to create context: %u\n", clRet);
		return err;
	}

	clQueue = clCreateCommandQueue(clContext, devices[0], 0, &clRet);
	if (clQueue == nullptr) {
		printf("Unable to create command queue: %u\n", clRet);
		return err;
	}

	clBuff = clCreateBuffer(clContext, (read_only ? CL_MEM_READ_ONLY : CL_MEM_READ_WRITE) | CL_MEM_ALLOC_HOST_PTR, targetSize, nullptr, &clRet);
	if (clBuff == nullptr) {
		printf("Unable to create memory buffer: %u\n", clRet);
		return err;
	}
#endif

	*size = targetSize;

	return (void*)1;
}
