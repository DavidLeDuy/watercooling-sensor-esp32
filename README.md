# DIY-watercooling

Watercooling with ESP32 + Alphacool Eiszapfen temperature sensor 17596 (NTC)

VSCode + PlatformIO

Fan Control

[R-T Table](https://download.alphacool.com/legacy/kOhm_Sensor_Table_Alphacool.pdf) for calculating the temperature from the differing resistance.


Output is then read by the _WatchLiquidTemp.ps1_ or the rust-watcher script which takes the Port that is used by the ESP32 to connect to the PC and a file path to a writable file.

The file can then be used by the Fan Control software to get the water temperature.


## PowerShell script

PowerShell code to watch a serial port and write the temperature to a file.
To make it run at startup, you can add it to the scheduled tasks.

```bash
-WindowStyle Hidden -File "E:\Git\ESP32 Scripts\WatchLiquidTemp.ps1" -filePath "D:\FanControl_net_7_0\LiquidTemp.sensor" -com COM3
```

## Rust watcher

Rust code to watch a serial port and write the temperature to a file.

```bash
# Build the project
rust-watcher\target\release\rust-watcher.exe --file "C:\Path\To\FanControl\sensor" --com "COM3"
```

This will watch the serial port COM3 and write the temperature to the file where Fan Control software is looking for it.

### Running at Startup

#### Windows Task Scheduler (Recommended)
Since the watcher is a standard console application, the easiest way to run it at startup is via the **Windows Task Scheduler**:
1. Open Task Scheduler and click **Create Basic Task**.
2. Set the Trigger to **When the computer starts** or **When I log on**.
3. Set the Action to **Start a program**.
4. Set Program/script to:
   `E:\Git\watercooling-sensor-esp32\rust-watcher\target\release\rust-watcher.exe`
5. Set Add arguments to:
   `--file D:\FanControl_net_7_0\LiquidTemp.sensor --com COM3`
6. Finish the wizard. In the task properties, check **Run whether user is logged on or not** and check **Hidden** to keep the terminal window hidden.

