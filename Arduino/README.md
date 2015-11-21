#Arduino

This folder contains some sketches used with the Arduino to achieve
certain functions.

## 20mA Konverter
The 20mA Konverter sketch translates a 20mA power equivalent of a 
current clamp to a S0 compliant impulse used to monitor the output
of a wind turbine. In its default configuration 1 impulse is generated
per 50 Wh. Also a non-zero offset power is subtracted (reactive power).

## Uno NTP receiver
The Uno NTP ENC28J60 sketch sends Network Time Protocol packages to a
time server and receives the corresponding time packages via UDP; time
is output to the serial connection. This sketch works with an Arduino Uno
and attached ENC28J60 module using the Arduino UIP library.