#RS232 to 485 Converter

The two boards in this project are the result of a test when we got our
PV DC-AC converter installed. It has a RS-485 interface to read it out with
some software provided by the vendor - but no PC had a RS-485 interface, but
"pure old legacy RS-232". After some research there are two solutions to
interfacing RS-485 with "legacy" RS-232.

<img src="RS232-RS485-Conv.jpg" width=400px>

1) RS232-RS485 is a converter that uses RTS/CTS for flow control; the transistor
   is used to configure actual flow control (on high or on low, depending on the
   terminal program used)

2) RS485auto is a converter that creates flow control from the baudrate and
   a defined length a character will take to transfer; to toggle the write
   line with the "correct timing" an ATtiny13 is used to delay for character length.

The ATtiny13 variant is inspired by an Elektor-version of the same kind of
converter that instead of an Atmel chip uses a PIC microcontroller. But as I am
not in the PIC business...

Note that today an FTDI FT 232 offers the possibility to interface RS-485 directly
to USB...

Markus Gebhard, Karlsruhe, Germany, Copyright 2009-2012 - all under GPL...