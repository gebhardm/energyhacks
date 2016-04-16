/*******************************************************
This is a fake sensor; it connects to an mDNS advertised
MQTT broker and continously sends data messages on
topic /sensor/fakesensor/gauge of the form 
[timestamp, value, unit]
*******************************************************/
var mqtt = require("mqtt");

var mdns = require("mdns");

// resolution requence added due to mdns issue - see https://github.com/agnat/node_mdns/issues/130
var sequence = [ mdns.rst.DNSServiceResolve(), "DNSServiceGetAddrInfo" in mdns.dns_sd ? mdns.rst.DNSServiceGetAddrInfo() : mdns.rst.getaddrinfo({
    families: [ 4 ]
}), mdns.rst.makeAddressesUnique() ];

// detect mqtt publishers and create corresponding servers
var mdnsbrowser = mdns.createBrowser(mdns.tcp("mqtt"), {
    resolverSequence: sequence
});

// handle detected mqtt brokers
mdnsbrowser.on("serviceUp", function(service) {
    console.log("Detected MQTT service on: " + service.addresses[0] + ":" + service.port);
    handle_mqtt_service(service.addresses[0], service.port);
});

// handle if mdns service goes offline
mdnsbrowser.on("serviceDown", function(service) {
    console.log("MDNS service went down: ", service);
});

// start the mdns browser
mdnsbrowser.start();

// handle the detected mqtt service
function handle_mqtt_service(address, port) {
    mqttclient = mqtt.connect({
        port: port,
        host: address
    });
    mqttclient.on("connect", function() {
        var now = new Date();
        console.log(now + " : Connected to " + address + ":" + port);
        //mqttclient.subscribe("/sensor/+/gauge");
        setInterval(send_messages, 1e3);
    });
    mqttclient.on("error", function() {
        // error handling to be a bit more sophisticated...
        console.log("An MQTT error occurred...");
    });
    mqttclient.on("message", function(topic, message) {
        var topicArray = topic.split("/");
        var payload = message.toString();
        console.log(payload);
    });
}

// send messages
function send_messages() {
    var payload = new Array();
    var date = new Date();
    var now = parseInt(date.getTime() / 1e3);
    var value = parseInt(date.getSeconds() * 6);
    payload.push(now);
    payload.push(Math.round(100 + 100 * Math.sin(2 * Math.PI * value / 360)));
    payload.push('"W"');
    mqttclient.publish("/sensor/fakesensor/gauge", "[" + payload.toString() + "]");
}