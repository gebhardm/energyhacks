#!/usr/bin/env node
/***********************************************************
This .js file subscribes to FLM sensor topics
and stores the received sensor gauges into a
mysql database.
(c) Markus Gebhard, Karlsruhe, 2014 - under MIT license
delivered "as is", no guarantee to work ;-)

uses
  mysql module: https://github.com/felixge/node-mysql
  mqtt module: https://github.com/adamvr/MQTT.js/
************************************************************/
var mysql = require('mysql');
var mqtt  = require('mqtt');

var mqttbroker = '192.168.0.50'; // provide your broker's address
var mqttport   = 1883;           // the default port
var mqttclient = mqtt.createClient(mqttport, mqttbroker);

var database = mysql.createConnection(
   {
      host : 'localhost',
      user : 'pi',
      password : 'raspberry',
      database : 'flm'
   }
);

mqttclient.on('message', function(topic, payload) {
   var subtopics = topic.split('/');
   switch (subtopics[3]) {
      case 'gauge':
         var gauge = JSON.parse(payload);
// FLM gauges consist of timestamp, value, and unit
         if (gauge.length == 3) {
            var date = new Date(gauge[0]*1000).toISOString().slice(0, 19).replace('T', ' ');
            var insertStr = 'INSERT INTO flmdata'
                          + ' (sensor, timestamp, value, unit)'
                          + ' VALUES ("' + subtopics[2] + '",'
                          + ' "' + date + '",'
                          + ' "' + gauge[1] + '",'
                          + ' "' + gauge[2] + '")'
                          + ' ON DUPLICATE KEY UPDATE'
                          + ' sensor = VALUES(sensor),' 
                          + ' timestamp = VALUES(timestamp),'
                          + ' value = VALUES(value),'
                          + ' unit = VALUES(unit);';
            database.query(insertStr, function(err, res) {
              if (err) {
                database.end();
                throw err;
              }
            });
         }
         break;
      case 'counter': break;
   }
});

database.connect(function(err) {
  if (err) throw err;
  console.log('Database flm successfully connected');
});

// create the persistence 
var createTabStr = 'CREATE TABLE IF NOT EXISTS flmdata'
                 + '( sensor CHAR(32),'
                 + '  timestamp TIMESTAMP,'
                 + '  value CHAR(5),'
                 + '  unit CHAR(5),'
                 + '  UNIQUE KEY (sensor, timestamp));';
database.query(createTabStr, function(err, res) {
   if (err) {
      database.end(); 
      throw err;
   }
   console.log('Create table successful...');
});

mqttclient.subscribe('/sensor/#');
