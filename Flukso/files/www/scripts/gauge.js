/* ************************************************************
   Display gauges for the Fluksometer
   code in parts taken from the mqttws31.js example and
   jpmens.net: 
   http://jpmens.net/2014/07/03/the-mosquitto-mqtt-broker-gets-websockets-support/ 
************************************************************ */
// objects containing the actual sensor data as string and value
var gauge = {}, displays = {};
// create an array of sensor values to pass on to a graph

var numgauge = 0;
var limit = 0;
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

			gauge[sensor] = value;
			unit = '';
		} else {
			switch (value.length) {
			case 1:

				gauge[sensor] = value[0];
				unit = '';
				break;
			case 2:

				gauge[sensor] = value[0];
				unit = value[1];
				break;
			case 3:
				var date = new Date(value[0] * 1000); // the timestamp

				var now = new Date().getTime();
				if ((now / 1000 - value[0]) > 60)
					value[1] = 0; // if too old, set to 0
				gauge[sensor] = value[1];
				unit = value[2];
				break;
			default:
				break;
			}
		}
		// create and fill an array of last n gauge
		// also create the corresponding table row to show - only if it not yet exists
		if (displays[sensor] == null) {
			numgauge++;
			// put always two gauges into one table row
			var tabcell = '<div id="' + sensor + '"></div>';
			if (numgauge % 2 == 1) {
				var tabrow = '<tr>' +
					'<td id="gc' + numgauge + '" width=50%></td>' +
					'<td id="gc' + (numgauge + 1) + '" width=50%></td>' +
					'</tr>';
				$('#gauge').append(tabrow);
			};
			$('#gc' + numgauge).append(tabcell);
			if (unit == 'W')
				limit = 250;
			else if (unit == '°C')
				limit = 50;
			else
				limit = 100;
			displays[sensor] = new JustGage({
					id : sensor,
					value : gauge[sensor],
					title : sensor,
					label : unit,
					min : 0,
					max : (gauge[sensor]>limit?gauge[sensor]:limit)
				});
			$('#sensor' + sensor).html('(Sensor ' + sensor + ')');
		};

		// now pass the data to the html part
		if (gauge[sensor] > displays[sensor].txtMaximum)
			displays[sensor].refresh(displays[sensor].originalValue, gauge[sensor]);
		else
			displays[sensor].refresh(gauge[sensor]);
		break;
	case 'counter':

		break;
	default:
		break;
	}
};

$(document).ready(function() { MQTTconnect(); });
