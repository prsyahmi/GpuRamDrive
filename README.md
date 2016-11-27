# GpuRamDrive
Create a virtual drive backed by GPU RAM.

This application simply allocates a memory buffer inside GPU RAM and use it as virtual ram disk. The project is made possible by ImDisk and its proxy feature.

Using GPU RAM isn't as fast as host main memory, however it is still faster than a regular HDD. The result below is taken on my system with GTX 850M and i7-4710MQ. As IO operation is happening between the CPU and GPU, the GPU can become active from idle state and might causes system to draw more power. This merely just a PoC, user who search for this kind of solution is advised to upgrade the RAM or buy a faster storage.

![Benchmark](https://cloud.githubusercontent.com/assets/1040494/20632692/65470470-b37a-11e6-908d-e08687a757d3.png)

People who interested in this might also want to check BadMemory: https://github.com/prsyahmi/BadMemory


## Compiling
To compile, open the solution, configure the target platform and build the project.

## Usage
1. Install ImDisk Virtual drive
2. Download DevIo from http://www.ltr-data.se/opencode.html/
3. Put the output dll, DevIo and batch files on a same folder
4. Run 1-RunGpuRamDrive.bat and 2-MountGpuRamDrive.bat
5. To unmount, run 3-UnMountGpuRamDrive.bat

By using the batch file, the memory size allocated is defaulted to 1.5GB since most PCs nowadays have at least 2GB RAM available on their GPU. To change simply edit the batch file.

The platform and device is also hardcoded, people who have multiple GPU might need to tweak them to use the correct one.

## License
This project is licensed under MIT. See LICENSE.
