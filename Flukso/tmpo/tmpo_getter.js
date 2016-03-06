/*
*  Subscribe to Fluksometer tmpo broadcast and show what was transmitted
*
*  Markus Gebhard, Karlsruhe, February 2015
*  under MIT license
*/
// get MQTT handling
var mqtt = require("mqtt");

// get the multi-domain-name-service w/ Bonjour detection
var mdns = require("mdns");

// get the decompression handling
var zlib = require("zlib");

// instantiate the Bonjour browser ...
var mdnsbrowser = mdns.createBrowser(mdns.tcp("mqtt"));

// ... and start it
mdnsbrowser.start();

// if a service is up, deal with it
mdnsbrowser.on("serviceUp", function(service) {
    mdnsservice(service);
});

function mdnsservice(service) {
    console.log("Detected MQTT service on: " + service.addresses[0] + ":" + service.port);
    // log on to the MQTT service
    var mqttclient = mqtt.createClient(service.port, service.addresses[0]);
    // and subscribe to the tmpo broadcast
    mqttclient.subscribe("/sensor/+/tmpo/#");
    // if a message is received, deal with it ...
    mqttclient.on("message", function(topic, payload) {
        mqtthandler(topic, payload);
    });
    // ... by decompressing the payload and showing it
    function mqtthandler(topic, payload) {
        console.log(topic);
        zlib.gunzip(payload, function(err, ts) {
            var json = JSON.parse(ts);
            console.log(json);
        });
    }
}