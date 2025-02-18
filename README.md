[Getting started](#getting-started)  
[Flashing](#flashing)  

# Getting started
This project uses VS Code and [Dev Containers](https://code.visualstudio.com/docs/devcontainers/containers) for development and building.
1. Install Visual Studio Code.
2. Enabled [Dev Containers tutorial](https://code.visualstudio.com/docs/devcontainers/tutorial) and open the project in the remote environment.
3. Click the `Build` button to compile the project.

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
