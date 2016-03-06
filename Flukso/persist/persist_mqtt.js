/***********************************************************
This .js file subscribes to FLM sensor topics
and stores the received sensor gauges into a
mysql database.
(c) Markus Gebhard, Karlsruhe, 2014/2015 - under MIT license
delivered "as is", no guarantee to work ;-)

uses dependencies defined in package.json
************************************************************/
var mysql = require("mysql");

// database connect and query
var mqtt = require("mqtt");

// mqtt message client
var mdns = require("mdns");

// multicast DNS service discovery
var mdnsbrowser = mdns.createBrowser(mdns.tcp("mqtt"));

// the database to use
var database;

// detect the MQTT broker using Bonjour
mdnsbrowser.on("serviceUp", function(service) {
    console.log("detected:" + service.addresses[0] + ":" + service.port);
    // connect to discovered mqtt broker
    var mqttclient = mqtt.connect({
        port: service.port,
        host: service.addresses[0]
    });
    // subscribe to fluksometer topics
    mqttclient.on("connect", function() {
        mqttclient.subscribe("/device/+/config/sensor");
        mqttclient.subscribe("/sensor/#");
    });
    mqttclient.on("error", function() {
        console.log("MQTT client raised an error...");
    });
    // act on received message
    mqttclient.on("message", function(topic, message) {
        var topicArray = topic.split("/");
        var payload = message.toString();
        // don't handle messages with weird tokens, e.g. compression
        try {
            payload = JSON.parse(payload);
        } catch (error) {
            return;
        }
        switch (topicArray[1]) {
          case "device":
            handle_device(topicArray, payload);
            break;

          case "sensor":
            handle_sensor(topicArray, payload);
            break;

          default:
            break;
        }
    });
    // handle the device configuration
    function handle_device(topicArray, payload) {
        switch (topicArray[3]) {
          case "config":
            for (var obj in payload) {
                var cfg = payload[obj];
                if (cfg.enable == "1") {
                    var insertStr = "INSERT INTO flmconfig" + " (sensor, name)" + ' VALUES ("' + cfg.id + '",' + ' "' + cfg.function + '")' + " ON DUPLICATE KEY UPDATE" + " sensor = VALUES(sensor)," + " name = VALUES(name);";
                    database.query(insertStr, function(err, res) {
                        if (err) {
                            database.end();
                            throw err;
                        }
                    });
                    console.log("Detected sensor " + cfg.id + " (" + cfg.function + ")");
                }
            }
            break;

          default:
            break;
        }
    }
    // handle the sensor readings 
    function handle_sensor(topicArray, payload) {
        switch (topicArray[3]) {
          case "gauge":
            // FLM gauges consist of timestamp, value, and unit
            // you may define further gauge lengths to be persisted
            if (payload.length == 3) {
                var insertStr = "INSERT INTO flmdata" + " (sensor, timestamp, value, unit)" + ' VALUES ("' + topicArray[2] + '",' + ' "' + payload[0] + '",' + ' "' + payload[1] + '",' + ' "' + payload[2] + '")' + " ON DUPLICATE KEY UPDATE" + " sensor = VALUES(sensor)," + " timestamp = VALUES(timestamp)," + " value = VALUES(value)," + " unit = VALUES(unit);";
                database.query(insertStr, function(err, res) {
                    if (err) {
                        database.end();
                        throw err;
                    }
                });
            }
            break;

          case "counter":
            break;
        }
    }
});

function prepare_database() {
    // connect to database
    database = mysql.createConnection({
        host: "localhost",
        user: "pi",
        password: "raspberry",
        database: "flm"
    });
    database.connect(function(err) {
        if (err) throw err;
        console.log("Database 'flm' successfully connected");
    });
    // create the config persistence if it does not exist
    var createTabStr = "CREATE TABLE IF NOT EXISTS flmconfig" + "( sensor CHAR(32)," + "  name CHAR(32)," + "  UNIQUE KEY (sensor)" + ");";
    database.query(createTabStr, function(err, res) {
        if (err) {
            database.end();
            throw err;
        }
        console.log("Table 'flmconfig' created successfully...");
    });
    // create the data persistence if it does not exist
    createTabStr = "CREATE TABLE IF NOT EXISTS flmdata" + "( sensor CHAR(32)," + "  timestamp CHAR(10)," + "  value CHAR(5)," + "  unit CHAR(5)," + "  UNIQUE KEY (sensor, timestamp)," + "  INDEX idx_time (timestamp)" + ");";
    database.query(createTabStr, function(err, res) {
        if (err) {
            database.end();
            throw err;
        }
        console.log("Table 'flmdata' created successfully...");
    });
}

// check and, if necessary, create the database table
prepare_database();

// mdnsbrowser.on and start to discover
mdnsbrowser.start();