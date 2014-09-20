#DS18x20 to MQTT

This sketch reads the value from Dallas/Maxim DS18x20 one-wire temperature sensors and sends corresponding MQTT messages to the Fluksometer (or any other) MQTT broker. The temperature sensors are automatically detected using the referred example code - license as provided. 
The result is directly integrated into the panel output provided in the Raspberry section.

<img src="temperature_gauge.png" width=600px>

Note: I tried conditional compilation to switch between the different includes to be used for an original Arduino Ethernet and the "cheap" AVRNetIO variant, but the preprocessor seems to neglect an #if before an #include - so make sure to uncomment the right Ethernet-include!