# GpuRamDrive
Create a virtual drive backed by GPU RAM.

This application simply allocates a memory buffer inside GPU RAM and use it as virtual ram disk. The project is made possible by ImDisk and its proxy feature.

## Compiling
To compile, open the solution, configure the target platform and build the project.

## Usage
1. Install ImDisk Virtual drive
2. Download DevIo from http://www.ltr-data.se/opencode.html/
3. Put the output dll, DevIo and batch files on a same folder
4. Run 1-RunGpuRamDrive.bat and 2-MountGpuRamDrive.bat
5. To unmount, run 3-UnMountGpuRamDrive.bat

## License
This project is licensed under MIT. See LICENSE
