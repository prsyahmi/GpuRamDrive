@echo off
set SIZE_IN_BYTES=1572864000
devio --dll=GpuRamDrive.dll;GpuRamDrive shm:GpuRamDrive1 %SIZE_IN_BYTES%