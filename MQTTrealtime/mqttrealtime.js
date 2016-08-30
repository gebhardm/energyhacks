/*
 * This is script to serve a graph display on received MQTT messages
 * topic: /device/<device_id>/flx/voltage/<port_no> and
 *        /device/<device_id>/flx/current/<port_no>
 * payload: [[<timestamp>,<???>],[<32 comma separated values>],"mV|mA"]
 * Markus Gebhard, Karlsruhe, 2016
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
var mqttbroker = "192.168.0.55", mqttport = "1883";

var mqttclient = mqtt.connect({
    host: mqttbroker,
    port: mqttport
});

var http = require("http").createServer(httphandler).listen(httpport);

var fs = require("fs");

var url = require("url");

var path = require("path");

var io = require("socket.io")(http);

// pass mqtt messages to connected websocket
io.on("connection", function(socket) {
    console.log("WebSocket connected");
});

// check MQTT connection
mqttclient.on("connect", function() {
    //mqttclient.subscribe("/device/+/flx/voltage/+");
    mqttclient.subscribe("/device/+/flx/current/+");
    console.log("Connected: ", mqttbroker, ":", mqttport);
});

// log error from MQTT client
mqttclient.on("error", function() {
    console.log("The MQTT client raised an error ...");
});

mqttclient.on("message", function(topic, message) {
    var payload, phase, topicArray;
    topicArray = topic.split("/");
    phase = topicArray[topicArray.length - 1];
    payload = message.toString();
    try {
        payload = JSON.parse(payload);
    } catch (error) {
        console.log("Error parsing JSON");
        return;
    }
    if (payload[2] === "mV") {
        var series = payload[1];
        for (var val in series) series[val] = series[val] / 1e3;
        payload[2] = "V";
    }
    io.sockets.emit("load", {
        phase: phase,
        data: payload[1]
    });
});

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
        if (fs.statSync(filename).isDirectory()) filename += "/index.html";
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