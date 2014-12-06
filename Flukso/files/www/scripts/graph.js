$(function() {
	// allow tooltip on datapoints
	$("<div id='tooltip'></div>").css({
		position : "absolute",
		display : "none",
		border : "1px solid #ccc",
		padding : "2px",
		opacity : 0.90
	}).appendTo("body");
	// set plot area boundaries
	var offset = 20; //px
	var width = $(document).width() - offset * 2;
	var height = width * 3 / 4;
	height = (height>600?600:height);
	$("#graph").width(width).height(height).offset({
		left : offset
	});
	// compute hover
	$("#graph").on("plothover", function (event, pos, item) {
		if (item) {
			var itemTime = new Date(item.datapoint[0]);
			var hrs = itemTime.getHours();
			hrs = (hrs < 10 ? '0' + hrs : hrs);
			var min = itemTime.getMinutes();
			min = (min < 10 ? '0' + min : min);
			var sec = itemTime.getSeconds();
			sec = (sec < 10 ? '0' + sec : sec);
			$("#tooltip").html(hrs + ':' + min + ':' + sec + ' : ' + item.datapoint[1])
			.css({
				top : item.pageY + 7,
				left : item.pageX + 5
			})
			.fadeIn(200);
		} else
			$("#tooltip").hide();
	});
	// prepare graph display
	var series = new Array(); // the received values
	var selSeries = new Array(); // the selected series to show
	var color = 0;
	var options = {
		series : {
			lines : {
				show : true,
				steps : true
			},
			points : {
				show : false
			}
		},
		grid : {
			hoverable : true
		},
		xaxis : {
			mode : "time",
			timezone : "browser"
		},
		yaxis : {
			min : 0
		}
	};
	// link to the web server's IP address for MQTT socket connection
	var client = new Paho.MQTT.Client(location.host, 8083, "", "FLMgauge");
	// define callback routines
	client.onConnect = onConnect;
	client.onConnectionLost = onConnectionLost;
	client.onMessageArrived = onMessageArrived;
	// connect to MQTT broker
	client.connect({
		onSuccess : onConnect
	});

	function onConnect() {
		client.subscribe("/sensor/#");
	};

	function onConnectionLost(responseObj) {
		if (responseObj.errorCode !== 0)
			console.log("onConnectionLost:" + responseObj.errorMessage);
	};

	function onMessageArrived(message) {
		// split the received message at the slashes
		var topic = message.destinationName.split('/');
		var payload = message.payloadString;
		// the sensor message type is the third value
		var msgType = topic[3];
		// pass the message topic and content to the html part
		$('#message').html(message.destinationName + ', ' + payload);
		var sensor = topic[2]; // the sensor ID
		var value = JSON.parse(payload); // the transferred payload
		// now compute the gauge
		switch (msgType) {
		case 'gauge':
			// process currently only the FLM delivered values with timestamp
			if (value.length == 3) {
				// check time difference of received value to current time
				// this is due to pulses being send on occurance, so potentially outdated
				var now = new Date().getTime();
				var diff = now / 1000 - value[0];
				// drop values that are older than 10 sec - as this is a realtime view
				if (diff > 100)
					break;
				// check if current sensor was already registered
				var obj = series.filter(function (o) {
						return o.label == sensor;
					});
				// flot.time requires UTC-like timestamps;
				// see https://github.com/flot/flot/blob/master/API.md#time-series-data
				var timestamp = value[0] * 1000;
				// ...if current sensor does not exist yet, register it
				if (obj[0] == null) {
					obj = {};
					obj.label = sensor;
					obj.data = [timestamp, value[1]];
					obj.color = color;
					color++;
					series.push(obj);
					// add graph select option
					$('#choices').append("<div class='checkbox'>"
						+ "<small><label>"
						+ "<input type='checkbox' id='" + sensor
						+ "' checked='checked'></input>" + sensor 
						+ "</label></small>"
						+ "</div>");
				}
				// ...otherwise, push the current value
				else {
					obj[0].data.push([timestamp, value[1]]);
					// move out values older than 5 minutes
					var limit = parseInt(obj[0].data[0]);
					diff = (timestamp - limit) / 1000;
					if (diff > 300) {
						var selGraph = new Array();
						for (var i in series) {
							var selObj = {};
							selObj.label = series[i].label;
							selObj.data = series[i].data.filter(function (v) {
									return v[0] > limit;
								});
							selGraph.push(selObj);
						}
						series = selGraph;
					}
				}
			} // if length
			break;
		default:
			break;
		}
		// check the selected checkboxes
		selSeries = [];
		$("#choices").find("input:checked").each(function () {
			var key = $(this).attr("id");
			var s = series.filter(function (o) {
					return o.label == key;
				});
			selSeries.push(s[0]);
		});
		// plot the selection
		$.plot("#graph", selSeries, options);
	};
});
