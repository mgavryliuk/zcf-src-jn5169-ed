- [Overview](#overview)
  - [DIOs definition](#dios-definition)
  - [Board](#board)
  - [Circuit diagrams](#circuit-diagrams)
- [Build](#build)

# Overview
[WXKG07LM](https://www.zigbee2mqtt.io/devices/WXKG07LM.html) - Wireless remote switch D1 (double rocker). It uses the same circuit board as the `WXKG06LM` but comes with left and right buttons soldered.

## DIOs definition
- Left LED - DIO 10
- Right LED - DIO 11
- Left button - DIO 12
- Right button - DIO 16

## Board
Version: LM15-WS R1.1</br>
![Board Front](/images/WXKG06LM_WXKG07LM/WXKG07LM_board_front.png)
![Board Back](/images/WXKG06LM_WXKG07LM/WXKG07LM_board_back.png)

## Circuit diagrams
LEDs circuit: </br>
![LEDs circuit](/images/WXKG06LM_WXKG07LM/leds_circuit.png)

Buttons circuit:</br>
![Buttons circuit](/images/WXKG06LM_WXKG07LM/buttons_circuit.png)

# Build
To build firmware for this device, select preset `WXKG07LM` in CMake configuration.
