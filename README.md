# DIY-watercooling

Watercooling with ESP32 + Alphacool Eiszapfen temperature sensor 17596 (NTC)

VSCode + PlatformIO

Fan Control

[R-T Table](https://download.alphacool.com/legacy/kOhm_Sensor_Table_Alphacool.pdf) for calculating the temperature from the differing resistance.

Output is then read by the _WatchLiquidTemp.ps1_ script which takes the Port that is used by the ESP32 to connect to the PC and a file path to a writable file.

The file can then be used by the Fan Control software to get the water temperature.
