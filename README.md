[![CodeFactor](https://www.codefactor.io/repository/github/muonpi/libmuonpi/badge)](https://www.codefactor.io/repository/github/muonpi/libmuonpi)
# libmuonpi
A support library for the muonpi project.

## Dependencies

## Building
The library uses CMake as build system. In order to build the libraries, create a build directory, execute cmake and make.

    mkdir build
    cd build
    cmake ../libmuonpi
    make

### Build options

## Package creation
If you want to create packages for the library on a debian based distribution, you can do so through cpack.

In the build directory, execute cpack. The packages will be created in the directory `builddirectory/output/packages/`.

Note that each individual library is exported as its own package. Along with the normal packages containing the shared object files, there are -dev versions of each package containing the header files.

**-dev packages will only be created in release mode. In debug mode, -dbg packages will be created.**

## libmuonpi-core

## libmuonpi-mqtt

## libmuonpi-rest

## libmuonpi-detector
