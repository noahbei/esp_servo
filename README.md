# Curtain Controller Project
## Overview
This project is a smart curtain controller built using an ESP32. The controller allows you to open and close your curtains remotely via Wi-Fi. The ESP32 handles the control logic, connects to a web interface for remote operation, and manages a servo motor to adjust the curtains' position. A magnetic encoder is used to track the servo position and ensure accurate bounds checking.

## Features
- Remote Control: Open and close your curtains using a web interface from anywhere in your home while you're connected to Wi-Fi.
- Manual Control: Physical buttons connected to the ESP32 allow for manual operation of the curtains.
- Position Control: The magnetic encoder ensures precise control over the curtain's position.
- Bounds Checking: The system uses the magnetic encoder to prevent the servo from moving beyond the defined limits.
- Wi-Fi Connectivity: The ESP32 connects to your home network to allow remote control.

## Hardware Requirements
- ESP32 Dev Kit (WROOM or similar)
- 360 degree servo motor
- AS5600 Magnetic encoder for position tracking
- Physical control buttons (optional)
- I2C OLED Screen
- 5V 2A Power supply (appropriate for your servo motor and ESP32)

## Installation
Clone the Repository:

```bash
git clone https://github.com/noahbei/esp_servo.git
cd esp_servo
```
### Configure Wi-Fi:
Update the ssid and password variables in the wifi-config.h file with your Wi-Fi credentials:
```cpp
const char* ssid = "Your_SSID";
const char* password = "Your_Password";
```

## Usage
- Remote Operation: Use the web interface to open, close, or set specific positions for your curtains.
- Manual Operation: Press the connected buttons to manually control the curtain's position.

## Code Structure
- main.cpp: The main code file, handling the setup and loop logic.
- wifi_setup.cpp: Handles Wi-Fi connectivity and web server setup.
- servo_control.cpp: Controls the servo's movement and position using the magnetic encoder.
- web_interface.cpp: Serves the web interface for remote control.

## Future Enhancements
- Integration with Smart Home Systems: Add support for smart home systems like Alexa or Google Assistant.
- Scheduled Control: Implement scheduling features to open or close curtains automatically at specific times.
- Advanced Position Control: Improve the algorithm for even finer position control and smoother movements.

## Contributing
If you wish to contribute to this project, please fork the repository and create a pull request with your changes.

## License
This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments
Thanks to the open-source community for various libraries and code examples that helped in the development of this project.
