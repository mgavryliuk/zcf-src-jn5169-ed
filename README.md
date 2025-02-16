[Getting started](#getting-started)  
[Build](#build)  
[Flashing](#flashing)  

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
1. Build using cmake
```
rm -rf build && cmake --preset ubuntu
cmake --build build
```

# Flashing
Unfirtunately flashing works **ONLY** under Windows.

## Links
[JN51xx Boot Loader Operation (JN-AN-1003)](https://www.nxp.com/docs/en/application-note/JN-AN-1003.pdf)<br>
[JN51xx Production Flash Programmer
User Guide (JN-UG-3099)](https://www.nxp.com/docs/en/user-guide/JN-UG-3099.pdf)</br>
[Support Resources for JN516x MCUs](https://www.nxp.com/products/wireless-connectivity/zigbee/support-resources-for-jn516x-mcus:SUPPORT-RESOURCES-JN516X-MCUS)

## Tools
| Name  | Part number |
| ------------- | ------------- |
| JN51xx Production Flash Programmer  | JN-SW-4107  |

## Steps
1. Find chip port
```
PS C:\NXP\ProductionFlashProgrammer> .\JN51xxProgrammer.exe -l
Available connections:
COM5
```
2. Check device config.</br>
It is impossible to flash new firmware to a chip with CRP_LEVEL2
```
PS C:\NXP\ProductionFlashProgrammer> .\JN51xxProgrammer.exe -V 0 -s COM5 --deviceconfig
  COM5: Detected JN5169 with MAC address 00:15:8D:00:01:B9:6A:FE
  COM5: Device configuration: JTAG_DISABLE_CPU,VBO_200,CRP_LEVEL1,EXTERNAL_FLASH_NOT_ENCRYPTED,EXTERNAL_FLASH_LOAD_ENABLE
```
3. Flash new frimware to the chip
```
PS C:\NXP\ProductionFlashProgrammer> .\JN51xxProgrammer.exe -V 0 -s COM5 -f D:\Programs\bstudio_nxp\workspace\Aqara_D1_ALT\build\src\AQARA_D1_ALT.bin
```
