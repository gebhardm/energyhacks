#!/usr/bin/env node
/**
 * Copyright (c) 2013, Fabian Affolter <fabian@affolter-engineering.ch>
 * Released under the MIT license. See LICENSE file for details.
 * Adapted to FLM by Markus Gebhard, Karlsruhe, 02/2014
 * Static http server part taken from Ryan Florence (rpflorence on github)
 * https://gist.github.com/rpflorence/701407
 * ************************************************************
 * note: this is a changed version as socket.io was changed...
 */

// use http for page serving, fs for getting the index.html
var http = require('http').createServer(handler).listen(1080);
var fs = require('fs');
var url = require('url');
var path = require('path');

// use mqtt for client, socket.io for push,
var mqtt = require('mqtt');
var io = require('socket.io')(http); // the socket listens on the http port

// define the mqtt broker to connect to and set up the client
//var mqttbroker = 'localhost';
var mqttbroker = '192.168.0.50';  // provide local FLM address here
var mqttport = 1883;
var mqttclient = mqtt.createClient(mqttport, mqttbroker);

// Subscribe to topic
io.on('connection', function (socket) {
  socket.on('subscribe', function (data) {
    mqttclient.subscribe(data.topic);
  });
// Push the message to socket.io
  mqttclient.on('message', function(topic, payload) {  
    socket.emit('mqtt',
      {'topic'  : topic,
       'payload' : payload
      }
    );
  });
});

// Serve the index.html page
function handler (req, res) {
//  easiest case: serve a static html page
//  res.writeHead(200, { 'Content-Type':'text/html'});
//  fs.createReadStream('./index.html').pipe(res);
// higher sophisticated case: serve complex html
    var uri = url.parse(req.url).pathname
        , filename = path.join(process.cwd(), uri);

    var contentTypesByExtension = {
        '.html': "text/html",
        '.css':  "text/css",
        '.js':   "text/javascript"
    };

    fs.exists(filename, function(exists) {
        if(!exists) {
          res.writeHead(404, {"Content-Type": "text/plain"});
          res.write("404 Not Found\n");
          res.end();
          return;
    }

    if (fs.statSync(filename).isDirectory()) filename += '/index_new.html';

    fs.readFile(filename, "binary", function(err, file) {
      if(err) {        
        res.writeHead(500, {"Content-Type": "text/plain"});
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
};
