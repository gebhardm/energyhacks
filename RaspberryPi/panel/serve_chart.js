#!/usr/bin/env node
/***********************************************************
This .js file serves a chart display from FLM sensor data
stored in a mysql database.
(c) Markus Gebhard, Karlsruhe, 2014 - under MIT license
delivered "as is", no guarantee to work ;-)

uses
  mysql module: https://github.com/felixge/node-mysql
  socket.io module: http://github.com/learnboost/socket.io
************************************************************/
var mysql = require('mysql');
var http = require('http').createServer(handler);
var fs = require('fs');
var url = require('url');
var path = require('path');

// prepare database connection
var database = mysql.createConnection(
   {
      host : 'localhost',
      user : 'pi',
      password : 'raspberry',
      database : 'flm'
   }
);

// connect to the database flm
database.connect(function(err) {
  if (err) throw err;
  console.log('Database flm successfully connected');
});

// set up http server on port 8080
http.listen(8080);

// Serve the http request
function handler (req, res) {
         var uri = url.parse(req.url).pathname;
         var filename = path.join(process.cwd(), uri);

         var contentTypesByExtension = {
            '.html': "text/html",
            '.css': "text/css",
            '.js': "text/javascript"
         };

         fs.exists(filename, function(exists) {
           if(!exists) {
             switch(req.url) {
               case '/query':
                 req.on('data', function(chunk) {
                   console.log(chunk.toString());
                 });
               default:
                 res.writeHead(404, {"Content-Type": "text/plain"});
                 res.write("404 Not Found\n");
                 res.end();
                 return;
                 break;
              }
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