## ESP32 OSC Button Send 
### *With AP Config and Double Reset Detector*

----------
*This project uses an ESP32 board to send OSC messages with a button press.*

----------
### Hardware:
This demo used a FREENOVE ESP32-WROVER-DEV controller with a breadboard and a button on pin 13.

----

### Setup:

- After uploading to the device it will enter config mode and broadcast an AP. 
  - *On your computer/device you should see a wifi called "esp32_osc".* 

- Once connected, a WiFiManager window will open > Configure WiFi

- Enter SSID, Password, OSC IP (device you're sending osc to), OSC Port, OSC Address, OSC Argument (optional), and Button Pin (GPIO).
   - If you leave the Static IP, Gateway and Subnet empty then the device will use DHCP to get an IP and connect.

- When finished click "Save" and the ESP32 will store the configuration and reset. On boot it will automatically connect to the stored SSID WiFi. The onboard Blue LED will flash when connecting and stay on if connected.
  - If after Save and the WiFi password is invalid and doesn't connect, then you can get back to the config screen by going to `192.168.4.1` in your browser.

- If the onboard reset button is pressed twice (not too fast), then the ESP32 will reset to enter config mode and broadcast an AP to change settings.

----

###  Example:
Trigger a QLab cue wirelessly with a button click.
- When the button is pressed it will send on OSC message `/go/2` to a computer with an IP `192.168.99.27` on the local network running QLab on port `53000` this syntax triggers a GO on Cue 2 in QLab.
