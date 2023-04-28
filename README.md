# wifiIO
FSR IO board featuring built-in WiFi chip for direct threshold control without host software. No software needs to be installed on the host (Stepmania) computer to alter the FSR thresholds. The USB interface is an Arduno Pro Micro, which handles the FSR inputs and lights output. The wifi interface is an ESP-12E board that communicates with the Arduino via I2C. The ESP-12E board runs as an access point that broadcasts its own wifi network. It hosts a webpage that allows you to read FSR values, write and read thresholds, and change the light colors, from a smartphone or any other connected device.

This board is designed specifically for use with 4-FSR ITG pads such as my [Metal Travel Pad](https://github.com/natologic/metalTravelPad).
