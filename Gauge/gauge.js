// objects containing the actual sensor data
var sensors = {}, numGauges = 0;

// link to the web server's IP address for socket connection
var socket = io.connect(location.host);

socket.on("connect", function() {
    socket.on("mqtt", function(msg) {
        // split the received message at the slashes
        var topic = msg.topic.split("/");
        var payload = msg.payload;
        switch (topic[1]) {
          case "device":
            handle_device(topic, payload);
            break;

          case "sensor":
            handle_sensor(topic, payload);
            // pass the message topic and content to the html part
            $("#message").html(msg.topic + ", " + msg.payload);
            break;

          default:
            break;
        }
    });
    // handle the received device information
    function handle_device(topic, payload) {
        var deviceID = topic[2];
        if (topic[3] == "config") {
            var config = JSON.parse(payload);
            for (var obj in config) {
                var cfg = config[obj];
                if (cfg.enable == "1") {
                    if (sensors[cfg.id] == null) {
                        sensors[cfg.id] = new Object();
                        sensors[cfg.id].id = cfg.id;
                        sensors[cfg.id].name = cfg.function;
                    } else sensors[cfg.id].name = cfg.function;
                }
            }
        }
    }
    // handle the received sensor information
    function handle_sensor(topic, payload) {
        // the retrieved sensor information
        var sensor = {};
        // the message type is the third value
        var msgType = topic[3];
        // the sensor ID
        var sensorId = topic[2];
        // the transferred payload
        var value = JSON.parse(payload);
        if (sensors[sensorId] == null) {
            sensors[sensorId] = new Object();
            sensor.id = sensorId;
            sensor.name = sensorId;
        } else sensor = sensors[sensorId];
        // now compute the gauge
        switch (msgType) {
          case "gauge":
            // Sensor handling - transfer the current values from the payload
            if (value.length == null) {
                sensor.value = value;
                sensor.unit = "";
            } else {
                switch (value.length) {
                  case 1:
                    sensor.value = value[0];
                    sensor.unit = "";
                    break;

                  case 2:
                    sensor.value = value[0];
                    sensor.unit = value[1];
                    break;

                  case 3:
                    var date = new Date(value[0] * 1e3);
                    // the timestamp
                    var now = new Date().getTime();
                    if (now / 1e3 - value[0] > 60) value[1] = 0;
                    // if too old, set to 0
                    sensor.value = value[1];
                    sensor.unit = value[2];
                    break;

                  default:
                    break;
                }
            }
            // create and fill an array of last n gauge
            // also create the corresponding table row to show - only if it not yet exists
            if (sensor.display == null) {
                numGauges++;
                // put always two gauges into one table row
                var tabcell = '<div id="' + sensorId + '"></div>';
                if (numGauges % 2 == 1) {
                    var tabrow = "<tr>" + '<td id="gc' + numGauges + '" width=50%></td>' + '<td id="gc' + (numGauges + 1) + '" width=50%></td>' + "</tr>";
                    $("#gauge").append(tabrow);
                }
                $("#gc" + numGauges).append(tabcell);
                var limit = 0, decimals = 0;
                if (sensor.unit == "W") limit = 250; else if (sensor.unit == "°C") {
                    limit = 50;
                    decimals = 2;
                } else limit = 100;
                limit = sensor.value > limit ? sensor.value : limit;
                sensor.display = new JustGage({
                    id: sensor.id,
                    value: sensor.value,
                    title: sensor.name,
                    label: sensor.unit,
                    min: 0,
                    max: limit,
                    decimals: decimals
                });
            }
            // now pass the data to the html part
            if (sensor.value > sensor.display.txtMaximum) {
                sensor.display.refresh(sensor.value, sensor.value);
            } else sensor.display.refresh(sensor.value);
            sensors[sensorId] = sensor;
            break;

          case "counter":
            break;

          default:
            break;
        }
    }
    // emit the subscription
    socket.emit("subscribe", {
        topic: "/device/#"
    });
    socket.emit("subscribe", {
        topic: "/sensor/#"
    });
});