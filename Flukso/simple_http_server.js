/**
 * Static http server part taken from Ryan Florence (rpflorence on github)
 * https://gist.github.com/rpflorence/701407
 */

// use http for page serving, fs for getting the *.html files
var httpport = 1080;
var http = require('http').createServer(handler).listen(httpport);
var fs = require('fs');
var url = require('url');
var path = require('path');

// Serve the index.html page
function handler(req, res) {
	var uri = url.parse(req.url).pathname,
	filename = path.join(process.cwd(), uri);

	var contentTypesByExtension = {
		'.html' : "text/html",
		'.css' : "text/css",
		'.js' : "text/javascript"
	};

	fs.exists(filename, function (exists) {
		if (!exists) {
			res.writeHead(404, {
				"Content-Type" : "text/plain"
			});
			res.write("404 Not Found\n");
			res.end();
			return;
		}

		if (fs.statSync(filename).isDirectory())
			filename += '/index.html';

		fs.readFile(filename, "binary", function (err, file) {
			if (err) {
				res.writeHead(500, {
					"Content-Type" : "text/plain"
				});
				res.write(err + "\n");
				res.end();
				return;
			}

			var headers = {};
			var contentType = contentTypesByExtension[path.extname(filename)];
			if (contentType)
				headers["Content-Type"] = contentType;
			res.writeHead(200, headers);
			res.write(file, "binary");
			res.end();
		});
	});
};
