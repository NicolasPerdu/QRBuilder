# QRBuilder
QRBuilder is a GUI, cross-platform, open-source project that generates QR codes from an input string.
The error correction can be specified.
You can then preview and save the resulting image in PNG.

## Build Instructions
To set up the repo, you need to do :
```
git clone
git submodule init
git submodule update
```
For the moment, we use one CMake list different for each platform, so rename the right file to CMakeLists.txt
```
mkdir build
cd build
ccmake ..
```
To build the project, you need to build and link [SDL3](https://github.com/libsdl-org/SDL)
