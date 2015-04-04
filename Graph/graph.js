// link to the web server's IP address for socket connection
var socket = io.connect(location.host);

// the received values
var series = new Array(), sensors = {};

// the selected series to show
var selSeries = new Array();

var color = 0;

var options = {
    series: {
        lines: {
            show: true,
            steps: true
        },
        points: {
            show: false
        }
    },
    grid: {
        hoverable: true
    },
    xaxis: {
        mode: "time",
        timezone: "browser"
    },
    yaxis: {
        min: 0
    }
};

// process socket connection
socket.on("connect", function() {
    socket.on("mqtt", function(msg) {
        // determine topic and payload
        var topic = msg.topic.split("/");
        var payload = msg.payload;
        switch (topic[1]) {
          case "device":
            handle_device(topic, payload);
            break;

          case "sensor":
            handle_sensor(topic, payload);
            break;

          default:
            break;
        }
    });
    // handle the device information
    function handle_device(topic, payload) {
        var deviceID = topic[2];
        if (topic[3] == "config") {
            var config = JSON.parse(payload);
            for (var obj in config) {
                var cfg = config[obj];
                if (cfg.enable == "1") {
                    var sensorId = cfg.id;
                    if (sensors[sensorId] == null) {
                        sensors[sensorId] = new Object();
                        sensors[sensorId].id = cfg.id;
                        sensors[sensorId].name = cfg.function;
                    } else sensors[sensorId].name = cfg.function;
                }
            }
        }
    }
    // handle the sensor information
    function handle_sensor(topic, payload) {
        var sensor = {};
        var msgType = topic[3];
        var sensorId = topic[2];
        if (sensors[sensorId] == null) {
            sensors[sensorId] = new Object();
            sensor.id = sensorId;
            sensor.name = sensorId;
        } else sensor = sensors[sensorId];
        var value = JSON.parse(payload);
        // now compute the gauge
        switch (msgType) {
          case "gauge":
            // process currently only the FLM delivered values with timestamp
            if (value.length == 3) {
                // check time difference of received value to current time
                // this is due to pulses being send on occurance, so potentially outdated
                var now = new Date().getTime();
                var diff = now / 1e3 - value[0];
                // drop values that are older than 10 sec - as this is a realtime view
                if (diff > 100) break;
                // check if current sensor was already registered
                var obj = series.filter(function(o) {
                    return o.label == sensor.name;
                });
                // flot.time requires UTC-like timestamps;
                // see https://github.com/flot/flot/blob/master/API.md#time-series-data
                var timestamp = value[0] * 1e3;
                // ...if current sensor does not exist yet, register it
                if (obj[0] == null) {
                    obj = {};
                    obj.label = sensor.name;
                    obj.data = [ timestamp, value[1] ];
                    obj.color = color;
                    color++;
                    series.push(obj);
                    // add graph select option
                    $("#choices").append("<div class='checkbox'>" + "<small><label>" + "<input type='checkbox' id='" + sensor.name + "' checked='checked'></input>" + sensor.name + "</label></small>" + "</div>");
                } else {
                    obj[0].data.push([ timestamp, value[1] ]);
                    // move out values older than 5 minutes
                    var limit = parseInt(obj[0].data[0]);
                    diff = (timestamp - limit) / 1e3;
                    if (diff > 300) {
                        var selGraph = new Array();
                        for (var i in series) {
                            var selObj = {};
                            selObj.label = series[i].label;
                            selObj.data = series[i].data.filter(function(v) {
                                return v[0] > limit;
                            });
                            selObj.color = series[i].color;
                            selGraph.push(selObj);
                        }
                        series = selGraph;
                    }
                }
            }
            // if length
            break;

          default:
            break;
        }
        // check the selected checkboxes
        selSeries = [];
        $("#choices").find("input:checked").each(function() {
            var key = $(this).attr("id");
            var s = series.filter(function(o) {
                return o.label == key;
            });
            selSeries.push(s[0]);
        });
        // plot the selection
        var width = $("#graphpanel").width();
        var height = width * 3 / 4;
        height = height > 600 ? 600 : height;
        $("#graph").width(width).height(height);
        $.plot("#graph", selSeries, options);
        // and store the sensor configuration
        sensors[sensorId] = sensor;
    }
    socket.emit("subscribe", {
        topic: "/device/+/config/sensor"
    });
    socket.emit("subscribe", {
        topic: "/sensor/+/gauge"
    });
});

$(function() {
    // allow tooltip on datapoints
    $("<div id='tooltip'></div>").css({
        position: "absolute",
        display: "none",
        border: "1px solid #ccc",
        padding: "2px",
        opacity: .9
    }).appendTo("body");
    // assign hover function
    $("#graph").on("plothover", function(event, pos, item) {
        if (item) {
            var itemTime = new Date(item.datapoint[0]);
            var hrs = itemTime.getHours();
            hrs = hrs < 10 ? "0" + hrs : hrs;
            var min = itemTime.getMinutes();
            min = min < 10 ? "0" + min : min;
            var sec = itemTime.getSeconds();
            sec = sec < 10 ? "0" + sec : sec;
            $("#tooltip").html(hrs + ":" + min + ":" + sec + " : " + item.datapoint[1]).css({
                top: item.pageY + 7,
                left: item.pageX + 5
            }).fadeIn(200);
        } else $("#tooltip").hide();
    });
});
