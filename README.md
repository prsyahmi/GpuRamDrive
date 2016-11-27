# GpuRamDrive
Create a virtual drive backed by GPU RAM.

![Screenshot](https://cloud.githubusercontent.com/assets/1040494/20649241/8bbf0838-b4f5-11e6-8f17-067d4ec30065.png)

This application simply allocates a memory buffer inside GPU RAM and use it as virtual ram disk. The project is made possible by ImDisk and its proxy feature.

Using GPU RAM isn't as fast as host main memory, however it is still faster than a regular HDD. The result below is taken on my system with GTX 850M and i7-4710MQ. As IO operation is happening between the CPU and GPU, the GPU can become active from idle state and might causes system to draw more power. This merely just a PoC, user who search for this kind of solution is advised to upgrade the RAM or buy a faster storage.

![Benchmark](https://cloud.githubusercontent.com/assets/1040494/20632692/65470470-b37a-11e6-908d-e08687a757d3.png)

People who interested in this might also want to check BadMemory: https://github.com/prsyahmi/BadMemory

## Usage
1. Install ImDisk Virtual drive
2. Download GpuRamDrive from https://github.com/prsyahmi/GpuRamDrive/releases or Compile from source
3. Run GpuRamDrive_x86.exe or GpuRamDrive_x64.exe according to your platform

## Compiling
To compile, open the solution, configure the target platform and build the project.

## License
This project is licensed under MIT. See LICENSE.
