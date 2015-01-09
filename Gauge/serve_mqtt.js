/*
 * This is script to serve a gauge display on received MQTT messages
 * topic: /sensor/<sensor_id>/gauge
 * payload: [<timestamp>, <value>, <unit>]
 * Markus Gebhard, Karlsruhe, 01/2015
 *
 * Released under the MIT license. See LICENSE file for details.
 *
 * Static http server part taken from Ryan Florence (rpflorence on github)
 * https://gist.github.com/rpflorence/701407
 * ************************************************************
 * Note: Use socket.io v1.0 for this script...
 */
// use http for page serving, fs for getting the *.html files
var httpport = 1080;

// use mqtt for client, socket.io for push,
var mqtt = require("mqtt");
var mqttclient;
// specify here your MQTT broker's data
var mqttbroker = '192.168.0.50', mqttport = '1883';

var http = require("http").createServer(httphandler).listen(httpport);

var fs = require("fs");

var url = require("url");

var path = require("path");

var io = require("socket.io")(http);

// store detected sensors
var sensors = {};

function mqttconnect() {
    mqttclient = mqtt.createClient(mqttport, mqttbroker);
    // handle socketio requests
    io.on("connection", function(socket) {
        // handle subscription request(s)
        socket.on("subscribe", function(data) {
            mqttclient.subscribe(data.topic);
        });
    });
    // handle mqtt messages
    mqttclient.on("message", function(topic, payload) {
        var topicArray = topic.split("/");
        switch (topicArray[1]) {

          case "sensor":
            handle_sensor(topicArray, payload);
            break;

          default:
            break;
        }
        // emit received message to socketio listener
        io.sockets.emit("mqtt", {
            topic: topic,
            payload: payload
        });
    });
    // handle the sensor readings
    function handle_sensor(topicArray, payload) {
        switch (topicArray[3]) {
			case "device":
				break;
			
			case "gauge":
            var gauge = JSON.parse(payload);
            // FLM gauges consist of timestamp, value, and unit
            if (gauge.length == 2) {
                // enhance payload w/o timestamp by current timestamp
                var now = parseInt(new Date().getTime() / 1e3);
                var new_payload = [];
                new_payload.push(now, gauge[0], gauge[1]);
                payload = JSON.stringify(new_payload);
            }
            // gauge length 2 is sent from Arduino sensors (in my case)
            break;

          case "counter":
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

mqttconnect();