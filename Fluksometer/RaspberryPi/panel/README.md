# FLM MQTT panel
An MQTT-panel adaptation to the flukso.net Fluksometer.
"serve_panel.js" is a JavaScript running on node.js. It serves as
a MQTT message to socket.io pipe with an integrated simple http
server. The provided page (index.html) displays data of all retrieved
FLM sensor gauges; the panel.html includes the counters also.

<img src="FLM_panel.png" width=400px>

# How to use
To run the script install node.js; easiest from http://github.com/joyent/node.
Install node.js using the command sequence

	git checkout v0.10.28-release
	./configure
	make
	sudo make install

Install the MQTT and socket.io modules with "npm install mqtt socket.io".
ATTENTION: socket.io has incompatibly changed between v0.9 and v1.0 - this panel
version does work with the v0.9 only; there is a new repository where I will
keep up with the evolution of the different modules; thus, this repo will stay
as it is... See [gebhardm/flmdisplay](http://github.com/gebhardm/flmdisplay)

On a running node.js installation just run the *./panel.sh* script to start
the server - be aware to adapt the MQTT client address in the *serve_panel.js*
to your local FLM's address.
Point your wherever located browser to the web server's IP address port 1080,
in my example *http://192.168.0.70:1080* - the *index.html* page is loaded
automatically; for the *panel.html* file use 

	http://<address>:1080/panel.html

Have fun.

With the greatest acknowledgements to Fabian Affolter and Ryan Florence...<br/>
The original MQTT panel by FabAff: [https://github.com/fabaff/mqtt-panel](https://github.com/fabaff/mqtt-panel)<br/>
HTTP server part by Ryan Florence: [https://gist.github.com/rpflorence/701407](https://gist.github.com/rpflorence/701407)<br/>

All code, corresponding to the sources, under MIT-license.

Note: The script and html works also in other environment with node.js is
installed; the above screenshot is actually made on my iMac...

Markus Gebhard, Karlsruhe, February 6, 2014/May 1, 2014
