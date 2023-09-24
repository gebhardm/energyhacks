/*
 * This is script to serve a gauge display on received MQTT messages
 * topic: /sensor/<sensor_id>/gauge
 * payload: [<timestamp>, <value>, <unit>]
 * Markus Gebhard, Karlsruhe, 2015-2016
 *
 * Released under the MIT license. See LICENSE file for details.
 *
 * Static http server part taken from Ryan Florence (rpflorence on github)
 * https://gist.github.com/rpflorence/701407
 * ************************************************************
 * Note: see package.json for dependencies
 */
// use http for page serving, fs for getting the *.html files
var httpport = 1080;

// use mqtt for client, socket.io for push,
var mqtt = require("mqtt");

// specify your MQTT broker's data here
// var mqttbroker = "192.168.0.50", mqttport = "1883";

// define the http server
var http = require("http").createServer(httphandler).listen(httpport);

var fs = require("fs");

var url = require("url");

var path = require("path");

var io = require("socket.io")(http);

// multicast DNS service discovery
var mdns = require("mdns");

// resolution requence added due to mdns issue - see https://github.com/agnat/node_mdns/issues/130
var sequence = [ mdns.rst.DNSServiceResolve(), "DNSServiceGetAddrInfo" in mdns.dns_sd ? mdns.rst.DNSServiceGetAddrInfo() : mdns.rst.getaddrinfo({
    families: [ 4 ]
}), mdns.rst.makeAddressesUnique() ];

// detect mqtt publishers and create corresponding servers
var mdnsbrowser = mdns.createBrowser(mdns.tcp("mqtt"), {
    resolverSequence: sequence
});

mdnsbrowser.on("serviceUp", function(service) {
    console.log("Detected MQTT service on: " + service.addresses[0] + ":" + service.port);
    mqttconnect(service.addresses[0], service.port);
});

mdnsbrowser.on("serviceDown", function(service) {
    console.log("MQTT service down: ", service);
});

mdnsbrowser.on("error", function(exception) {
    console.log("An error occured: ", exception);
});

// for manual broker definition, comment out the next line
// mqttconnect(mqttbroker, mqttport);

// start the mdns browser
mdnsbrowser.start();

// define what to do on connecting the mqtt broker
function mqttconnect(mqttbroker, mqttport) {
    var mqttclient;
    // create the MQTT connection
    mqttclient = mqtt.connect({
        port: mqttport,
        host: mqttbroker
    });
    // handle socketio requests
    io.on("connection", function(socket) {
        // handle subscription request(s)
        socket.on("subscribe", function(data) {
            mqttclient.subscribe(data.topic);
        });
    });
    // check MQTT connection
    mqttclient.on("connect", function() {
        console.log("Connected: ", mqttbroker, ":", mqttport);
    });
    // log error from MQTT client
    mqttclient.on("error", function() {
        console.log("The MQTT client raised an error ...");
    });
    // handle mqtt messages
    mqttclient.on("message", function(topic, message) {
        // split topic into array to check for sub-topics
        var topicArray = topic.split("/");
        // now deal with the payload, which is sent as BUFFER object
        var payload = message.toString();
        // don't handle messages with weird tokens, e.g. compression
        try {
            // deal with the JSON decoded payload from here
            payload = JSON.parse(payload);
        } catch (error) {
            return;
        }
        // compute different sub-topics here
        switch (topicArray[1]) {
          case "sensor":
            handle_sensor(topicArray, payload);
            break;

          default:
            break;
        }
        // emit received message to socket.io listener
        // a stringified JSON object is passed on
        io.sockets.emit("mqtt", {
            topic: topic,
            payload: JSON.stringify(payload)
        });
    });
    // handle the sensor readings
    function handle_sensor(topicArray, payload) {
        switch (topicArray[3]) {
          case "gauge":
            // FLM gauges consist of timestamp, value, and unit
            // gauge length 2 is sent from Arduino sensors (in my case):
            // [<value:float>,"<unit>:string"]
            switch (payload.length) {
              case 2:
                // enhance payload w/o timestamp by current timestamp
                var now = parseInt(new Date().getTime() / 1e3);
                payload.unshift(now);
                break;
            }
            break;

          default:
            break;
        }
    }
}

// Serve the index.html page
function httphandler(req, res) {
    var uri = url.parse(req.url).pathname, filename = path.join(process.cwd(), uri);
    var contentTypesByExtension = {
        ".html": "text/html",
        ".css": "text/css",
        ".js": "text/javascript"
    };
    // serve requested files
    fs.exists(filename, function(exists) {
        if (!exists) {
            res.writeHead(404, {
                "Content-Type": "text/plain"
            });
            res.write("404 Not Found\n");
            res.end();
            return;
        }
        if (fs.statSync(filename).isDirectory()) filename += "/gauge.html";
        fs.readFile(filename, "binary", function(err, file) {
            if (err) {
                res.writeHead(500, {
                    "Content-Type": "text/plain"
                });
                res.write(err + "\n");
                res.end();
                return;
            }
            var headers = {};
            var contentType = contentTypesByExtension[path.extname(filename)];
            if (contentType) headers["Content-Type"] = contentType;
            res.writeHead(200, headers);
            res.write(file, "binary");
            res.end();
        });
    });
}
