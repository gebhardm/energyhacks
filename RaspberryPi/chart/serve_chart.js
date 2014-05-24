#!/usr/bin/env node
/***********************************************************
This .js file serves a chart display from FLM sensor data
stored in a mysql database. (alternative version using socket
instead of post method)
(c) Markus Gebhard, Karlsruhe, 2014 - under MIT license
delivered "as is", no guarantee to work ;-)

uses
  mysql module: https://github.com/felixge/node-mysql
  socket.io module: http://github.com/learnboost/socket.io
************************************************************/
var mysql = require('mysql');
var http = require('http').createServer(handler).listen(8080);
var fs = require('fs');
var url = require('url');
var path = require('path');
var qs = require('querystring');
var socket = require('socket.io');

var dbaccess = {
      host : 'localhost',
      user : 'pi',
      password : 'raspberry',
      database : 'flm'
    };


// prepare websocket
var io = socket.listen(http);
io.set('log level', 1);

// Serve the http request
function handler (req, res) {
   var uri = url.parse(req.url).pathname;
   var filename = path.join(process.cwd(), uri);

   var contentTypesByExtension = {
      '.html': "text/html",
      '.css': "text/css",
      '.js': "text/javascript"
   };

// serve requested files
   fs.exists(filename, function(exists) {
      if(!exists) {
        res.writeHead(404, {"Content-Type": "text/plain"});
        res.write("404 Not Found\n");
        res.end();
        return;
      }

      if (fs.statSync(filename).isDirectory()) filename += '/chart.html';

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

io.sockets.on('connection', function(socket) {
  socket.on('query',  function (data) { handlequery(data); });
});

function handlequery(data) {
  var fromTimestamp = Date.parse(data.fromDate + ' ' + data.fromTime)/1000;
  var toTimestamp = Date.parse(data.toDate + ' ' + data.toTime)/1000;

// prepare database connection
  var database = mysql.createConnection(dbaccess);
  database.connect(function(err) {
    if (err) throw err;
  });
// fetch flm data from database
  var queryStr = 'SELECT * FROM flmdata WHERE timestamp >= \''
                  + fromTimestamp + '\' AND timestamp <= \''
                  + toTimestamp + '\';';
  var query = database.query(queryStr, function(err, rows, fields) {
    if (err) throw err;
    var series = {};
    for (var i in rows) {
      if (series[rows[i].sensor] == null) series[rows[i].sensor] = new Array();
        series[rows[i].sensor].push([rows[i].timestamp*1000,rows[i].value]);                    
    }
// send data to requester
    io.sockets.emit('series', series);
// ...and close the database again
    database.end();
  });
}
