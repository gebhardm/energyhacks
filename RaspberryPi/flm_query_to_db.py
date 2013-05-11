# Fluksometer - query sensors to database
# ----------------------------------------------------------------
# reads local FLM sensor information and passes them to MySQL database
# make sure to have MySQL installed; if not already, then
# execute sudo apt-get install mysql-server mysql-client
# provide a database named flm (or alter below code accordingly)
# for convenient access in Python2 (sic!) install the MySQLdb module
# with sudo apt-get install python-mysqldb
# for convenient http-communication install httplib2
# for that please refer to http://code.google.com/p/httplib2/

__author__ = "Markus Gebhard, Karlsruhe"
__copyright__ = "Copyright May 2013"
__credits__ = ["raspberrypi.org", "httplib2", "Simon Monk"]
__license__ = "GPL"
__version__ = "0.2"
__maintainer__ = "Markus Gebhard"
__email__ = "markus.gebhard@web.de"
__status__ = "draft"

# now the code

import httplib2, MySQLdb, json, time, sys
from datetime import datetime

# data definitions
# define your local sensor ids here - get them from flukso.net
sensors = ('b1a04f62f20a9acec3481cd90a357ae5',
           '830fefb0f0f898515b51266033e05fa6',
           'c486171ab3865fec2ffabe12cd24ee73',
           '0b5cf0a60f22093fade6ae86e2b561f8')
# define your local FLM's url here - see FLM manual for details
url = 'http://192.168.0.50:8080/sensor/'
# define local query (there is just one option so far
query = '?version=1.0&interval=minute&unit=watt'

# connect to database
try:
    db = MySQLdb.connect(host='localhost',
                         user='root',         # use your convenient user
                         passwd='raspberry',  # and password
                         db='flm')            # and database
except MySQL.Error, e:
    print 'Error %d: %s' % (e.args[0], e.args[1])
    sys.exit (1)

# prepare table to write data into
# create a table to store FLM values (if it does not exist)
try:
    cur = db.cursor()
    cur.execute("""
        CREATE TABLE IF NOT EXISTS flmdata
        (
            sensor    CHAR(32),
            timestamp TIMESTAMP, 
            power     CHAR(5),
            UNIQUE KEY (sensor, timestamp)
        )
        """)
except MySQLdb.Error, e:
    print "Error %d: %s" % (e.args[0], e.args[1])
    sys.exit (1)
    
# prepare querying data from local FLM
flm = httplib2.Http()
while True:
# query the sensors
    for sensor in sensors:
        req = url+sensor+query
        headers = { 'Accept':'application/json' }
        error = True
        while error:
            error = False
            try:
                # fetch data until read completely (had some errors here)
                response, content = flm.request(req, 'GET', headers=headers)
            except httplib2.HttpLib2Error: # IncompleteRead:
                error = True
            if response.status == 200:   
                data = json.loads(content)
            else:
                error = True
# save sensor data in database        
        for timestamp, power in data:
            try:
                # save values to database so that they occur only once each
                # for that update already read sensor data within the last
                # 30 seconds
                cur.execute("""INSERT INTO flmdata (sensor, timestamp, power)
                        VALUES (%s,%s,%s)
                        ON DUPLICATE KEY UPDATE
                        timestamp = VALUES(timestamp),
                        power = VALUES(power)""",
                        (sensor, str(datetime.fromtimestamp(timestamp)), power))
                db.commit()
            except MySQLdb.Error, e:
                print "Error %d: %s" % (e.args[0], e.args[1])
                sys.exit (1)
# wait for 30 seconds
    time.sleep(30)
# note - this is done in an infinite loop for now; will go for a cron job
# later...
