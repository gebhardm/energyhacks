# Raspberry Pi hacks

This folder contains some programs and scripts (in own folders) to be used
on a Raspberry Pi; in the first place all programs provided here are used 
to connect to a Fluksometer (see [flukso.net](http://www.flukso.net)), retrieve data, push data
to an MQTT broker, store data and visualize it.

* [chart/](chart/) retrieves sensor data from a database and shows it in a flotchart
* [panel/](panel/) contains an MQTT panel displaying sensor data via a node.js web server
* [persist/](persist/) actually is used to store FLM data in a database

ATTENTION: As node.js and its modules evolve, there are incompatible changes.
The scripts in this folder run under node.js v0.10.28-release with
socket.io in the version v0.9 - for keeping up with the evolution I moved to
a new repository: Get current script versions from
[gebhardm/flmdisplay](http://github.com/gebhardm/flmdisplay)

Markus Gebhard, Karlsruhe, 2014 (c), all code under MIT license if not
denoted otherwise.

Have fun...
