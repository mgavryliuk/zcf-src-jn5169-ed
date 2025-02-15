# Getting started
1. Register on [NXP](https://www.nxp.com)
2. Download and unpack Toolchain
    - [Ubuntu](https://github.com/openlumi/BA2-toolchain/releases/tag/20201219)
    - **Windows.** It is a part of the `BeyondStudio for NXP` IDE (JN-SW-4141).
        - go to [NXP Account](https://www.nxp.com/mynxp/home) -> Apps and Services -> Software Licensing and Support -> View accounts
        - download and install `JN-SW-4141`
        - copy folders from `<installation_path>/sdk/Tools/ba-elf-ba2-r36379` to `toolchain` folder.

# Build

## Ubuntu Docker image
Build is done in docker with CMake in docker container:
1. Build docker image:
```
docker build -f ubuntu-cmake.Dockerfile --rm --platform=linux/amd64 . -t ubuntu-cmake
```
2. Run cmake in docker image:
```
docker run --platform=linux/amd64 --rm -it -v .:/project ubuntu-cmake:latest

root@dfe47ad5042c:/project# rm -rf build && cmake --preset ubuntu
root@dfe47ad5042c:/project# cmake --build build
```
## Windows
TBA
