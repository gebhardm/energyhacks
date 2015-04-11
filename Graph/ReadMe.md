#Simple MQTT Graph

This hack is a tiny node.js based web server that subscribes to an MQTT topic from a sensor publisher and displays the received values as a continuous graph; the payload format used is

    [<timestamp:Unix-timestamp on second base>,<value:number>,"<unit:string>"]
    
If the payload is supplied without timestamp, the timestamp of the receiving time is added.

Actually this is a rip of the Fluksometer display to be found in [gebhardm/flmdisplay](https://github.com/gebhardm/flmdisplay/tree/master/combined)

<img src="MQTTgraph.png" width=500px>

To run the script, use

    node serve_mqtt.js
    
after having installed [node.js](http://nodejs.org) and the node modules **mqtt** and **socket.io**

    npm install mqtt socket.io
    
For convenience just use

    npm install
    
with the package.json file defining all necessary dependencies.

Access the served web page on port *1080* or whatever port you use in the [*serve_mqtt.js*](serve_mqtt.js).

    var httpport = 1080;
    
Be aware to also specify the MQTT broker you want to subscribe to.

    var mqttbroker = '192.168.0.50', mqttport = '1883';

The graph shows the last *300 seconds* of messages received; to alter this time interval, you may change the value of 300

    if (diff > 300) {

in [**graph.js**](graph.js).