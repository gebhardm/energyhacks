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
__version__ = "0.3.1"
__maintainer__ = "Markus Gebhard"
__email__ = "markus.gebhard@web.de"
__status__ = "draft"

# now the code

import httplib2, MySQLdb, json, time, sys
from datetime import datetime
# prepare logging to see what happens in the background
import logging

logging.basicConfig(filename='flm_query.log',
                    level=logging.DEBUG,
                    filemode='a',
                    format='%(asctime)s %(message)s',
                    datefmt='%m/%d/%Y %H:%M:%S')
logging.captureWarnings(True)

# data definitions
# define your local sensor ids here - get them from flukso.net
sensors = (('b1a04f62f20a9acec3481cd90a357ae5','L1'),
           ('830fefb0f0f898515b51266033e05fa6','L2'),
           ('c486171ab3865fec2ffabe12cd24ee73','L3'),
           ('0b5cf0a60f22093fade6ae86e2b561f8','PV'))
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
    handleError(e)

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
    handleError(e)
    
# prepare querying data from local FLM
flm = httplib2.Http()
while True:
# query the sensors
    for sensor, senid in sensors:
        req = url+sensor+query
        headers = { 'Accept':'application/json' }
        # try until fetched a valid result
        error = True
        while error:
            error = False
            try:
                # fetch data until read completely (had some errors here)
                response, content = flm.request(req, 'GET', headers=headers)
            except (httplib2.HttpLib2Error, httplib.IncompleteRead):
                error = True
            if response.status == 200:
                try:
                    data = json.loads(content)
                except ValueError:
                    error = True
            else:
                error = True
# save sensor data in database        
        for timestamp, power in data:
            try:
                if power == 'nan':
                    power = 0
                # save values to database so that they occur only once each
                # for that update already read sensor data within the last
                # 30 seconds
                cur.execute("""INSERT INTO flmdata (sensor, timestamp, power)
                        VALUES (%s,%s,%s)
                        ON DUPLICATE KEY UPDATE
                        timestamp = VALUES(timestamp),
                        power = VALUES(power)""",
                        (senid, str(datetime.fromtimestamp(timestamp)), power))
                db.commit()
            except MySQLdb.Error, e:
                handleError(e)
# wait for 30 seconds
    time.sleep(30)
# note - this is done in an infinite loop for now; will go for a cron job
# later...

# routine to handle errors
def handleError(e):
    print "Error %d: %s" % (e.args[0], e.args[1])
    logging.error('An error occured')
    sys.exit (1)
