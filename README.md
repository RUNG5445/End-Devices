# LoRa Sensor Node

This Arduino project utilizes LoRa communication to create a sensor node that measures temperature, humidity, and battery voltage. It sends the collected data to a LoRa gateway at predefined intervals.

## Components Used
- Arduino board (ESP32)
- LoRa module
- AHTX0 Temperature and Humidity Sensor
- Voltage divider circuit for battery monitoring

## Dependencies
- ArduinoJson library
- LoRa library
- Adafruit AHTX0 library

## Setup
1. Connect the AHTX0 sensor and the LoRa module to the Arduino board as per the wiring instructions.
2. Install the required libraries: ArduinoJson, LoRa, and Adafruit AHTX0.
3. Upload the provided code to your Arduino board.
4. Power up the Arduino board.

## Functionality
- Measures temperature and humidity using the AHTX0 sensor.
- Monitors battery voltage to track battery level.
- Sends sensor data to a LoRa gateway at regular intervals.
- Receives configuration parameters (SyncWord, TxPower, etc.) via LoRa to update settings dynamically.
- Utilizes deep sleep mode to conserve power between transmissions.

## Configuration
- The code can be configured with different LoRa settings such as SyncWord, TxPower, and frequency.
- Modify the `enddeviceslist` array to include the names of your end devices.
- Adjust the `defaultSyncWord`, `defaultTxPower`, `defaultfreq`, `defaultinterval`, `defaultSpreadingFactor`, and `defaultSignalBandwidth` variables to set default LoRa parameters.
- Define the `user` and `timeout` constants according to your requirements.

## Usage
- Ensure proper wiring and power supply for the Arduino board.
- Monitor serial output for debugging messages.
- Adjust LoRa settings and sensor readings as necessary.
- Customize the code for additional functionality or sensor integration.

## Authors

- [Rungrueng Janrueng](https://github.com/RUNG5445)

## License

This project is licensed under the [MIT License](LICENSE).

