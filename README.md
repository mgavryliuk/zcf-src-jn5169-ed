- [Overview](#overview)
- [Getting Started](#getting-started)
- [Devices](#devices)
- [JN5169 Documentation](#jn5169-documentation)

# Overview
This repository provides custom firmware implementations for JN5169 MCU-based devices.

Pre-built firmwares can be found in the [zigbee-custom-firmwares](https://github.com/mgavryliuk/zigbee-custom-firmwares) repository.

# Getting Started
This project uses VS Code and [Dev Containers](https://code.visualstudio.com/docs/devcontainers/containers) for development and building.
1. Install [Visual Studio Code](https://code.visualstudio.com/).
2. Follow the [Dev Containers tutorial](https://code.visualstudio.com/docs/devcontainers/tutorial) and open the project in the remote environment.
    - Use `.devcontainer/scripts/install-vscode-extensions.sh` to install extenstion into container if they were not installed automatically.
3. Download SDK `JN-SW-4170` from NXP site and add into sdk directory.
4. Click the `Build` button to compile the project.

# Devices
| Device | Description | Documentation | Build Preset |
|--------|-------------|---------------|--------------|
| WXKG06LM | Wireless remote switch D1 (single rocker) | [Documentation](firmwares/WXKG06LM/README.md) | `WXKG06LM` |
| WXKG07LM | Wireless remote switch D1 (double rocker) | [Documentation](firmwares/WXKG07LM/README.md) | `WXKG07LM` |
| WXKG11LM | Wireless mini switch | [Documentation](firmwares/WXKG11LM/README.md) | `WXKG11LM` |

# JN5169 Documentation
[Product page](https://www.nxp.com/products/JN5169)</br>
[Support Resources for JN516x MCUs](https://www.nxp.com/products/wireless-connectivity/zigbee/support-resources-for-jn516x-mcus:SUPPORT-RESOURCES-JN516X-MCUS)</br>
[JN516x/7x Zigbee 3.0](https://www.nxp.com/pages/jn516x-7x-zigbee-3-0:ZIGBEE-3-0)</br>
[Data Sheet: JN516x](docs/JN516X.pdf)</br>
[Data Sheet: JN5169](docs/JN5169.pdf)</br>
[IEEE 802.15.4 Stack (JN-UG-3024)](docs/JN-UG-3024.pdf)</br>
[ZigBee 3.0 Stack (JN-UG-3113)](docs/JN-UG-3113.pdf)</br>
[ZigBee 3.0 Devices (JN-UG-3114)](docs/JN-UG-3114.pdf)</br>
[ZigBee Cluster Library (for ZigBee 3.0) (JN-UG-3115)](docs/JN-UG-3115.pdf)</br>
[JN51xx Core Utilities (JN-UG-3116)](docs/JN-UG-3116.pdf)</br>
[JN516x Integrated Peripherals API (JN-UG-3087)](docs/JN-UG-3087.pdf)</br>
[JN51xx Production Flash Programmer (JN-UG-3099)](docs/JN-UG-3099.pdf)</br>
[JN51xx Boot Loader Operation (JN-AN-1003)](docs/JN-AN-1003.pdf)
