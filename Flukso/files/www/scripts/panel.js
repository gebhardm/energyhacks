// objects containing the actual sensor data as string and value
var sensors = {}, gauge = {}, counters = {};
// create an array of sensor values to pass on to a graph
var gaugeseries = {}, numgauge = 0, numcounter = 0;
// link to the web server's IP address for MQTT socket connection
var client;
var reconnectTimeout = 2000;

function MQTTconnect() {
	client = new Paho.MQTT.Client(location.host, 8083, "", "FLMgauge");
	var options = {
        	timeout : 3,
		onSuccess : onConnect,
		onFailure : function(message) { setTimeout(MQTTconnect, reconnectTimeout); }
	};
	// define callback routines
	client.onConnectionLost = onConnectionLost;
	client.onMessageArrived = onMessageArrived;
	client.connect(options);
};

function onConnect() {
	client.subscribe("/sensor/#");
};

function onConnectionLost(responseObj) {
	setTimeout(MQTTconnect, reconnectTimeout);
	if (responseObj.errorCode !== 0)
		console.log("onConnectionLost:" + responseObj.errorMessage);
};

function onMessageArrived(message) {
	// split the received message at the slashes
	var topic = message.destinationName.split('/');
	var payload = message.payloadString;
	// pass sensor message to the html part
	$('#message').html(message.destinationName + ", '" + payload);
	// the sensor message type is the third value of the topic
	var msgType = topic[3]; // gauge or counter
	var sensor = topic[2]; // the sensor ID
	var value = JSON.parse(payload); // the transferred payload
	var unit = '';
	// now compute the gauge
	switch (msgType) {
	case 'gauge':
		// Sensor handling - transfer the current values from the payload
		if (value.length == null) {
			sensors[sensor] = value;
			gauge[sensor] = value;
			unit = '';
		} else {
			switch (value.length) {
			case 1:
				sensors[sensor] = value[0];
				gauge[sensor] = value[0];
				unit = '';
				break;
			case 2:
				sensors[sensor] = value[0] + ' ' + value[1];
				gauge[sensor] = value[0];
				unit = value[1];
				break;
			case 3:
				var date = new Date(value[0] * 1000); // the timestamp
				sensors[sensor] = value[1] + ' ' + value[2] + ' (' + date.toLocaleTimeString("en-EN") + ')';
				gauge[sensor] = value[1];
				unit = value[2];
				break;
			default:
				break;
			}
		}
		// create and fill an array of last n gauge
		// also create the corresponding table row to show - only if it not yet exists
		if (gaugeseries[sensor] == null) {
			gaugeseries[sensor] = new Array();
			numgauge++;
			var tablerow = '<tr>' +
				'<td width = \"40%\" style=\"vertical-align:middle;\"><h4>Gauge ' + numgauge + '</h4>'
				 + '<small id=\"sensor' + sensor + '\">(no value received)</small></td>'
				 + '<td style=\"vertical-align:middle;\"><span id=\"valueSparkline' + sensor + '\">No values</span></td>'
				 + '<td width=\"30%\" style=\"vertical-align:middle;\"><h4><span id=\"value' + sensor + '\">Unknown</span></h4>'
				 + '<small id=\"cntr' + sensor + '\">(no value received)</small></td>'
				 + '</tr>';
			$('#gauge').append(tablerow);
		};
		if (gaugeseries[sensor].length == 60)
			gaugeseries[sensor].shift();
		gaugeseries[sensor].push(gauge[sensor]);
		// now pass the data to the html part
		$('#sensor' + sensor).html('Sensor ' + sensor);
		$('#cntr' + sensor).html('Total ' + counters[sensor]);
		$('#value' + sensor).html(sensors[sensor]);
		$('#valueSparkline' + sensor).sparkline(gaugeseries[sensor], {
			type : 'line',
			width : '200',
			height : '50',
			tooltipFormat : '<span class=\"text-info bg-info\">{{x}}:{{y}}</span>'
		});
		break;
	case 'counter':
		if (value[2] == 'Wh')
			counters[sensor] = value[1] / 1000.0 + ' kWh';
		else
			counters[sensor] = value[1] + ' ' + value[2];
		break;
	default:
		break;
	}
};

$(function() { MQTTconnect(); });
