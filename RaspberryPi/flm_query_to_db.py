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
__version__ = "0.3.3"
__maintainer__ = "Markus Gebhard"
__email__ = "markus.gebhard@web.de"
__status__ = "draft"

# now the code
# import http support
import httplib2, httplib
# import database support
import MySQLdb, json
# import relevant system functions
import time, sys
from datetime import datetime
# prepare logging to see what happens in the background
import logging, warnings

logging.basicConfig(filename='flm_query.log',
                    level = logging.ERROR, #level=logging.DEBUG,
                    filemode='a',
                    format='%(asctime)s %(message)s',
                    datefmt='%m/%d/%Y %H:%M:%S')
logging.captureWarnings(True)
# ignore warnings, e.g. that table flmdata exists...
warnings.filterwarnings('ignore')

# data definitions
# define your local sensor ids here - get them from flukso.net
# define a list of values (('<sensorid>','<sensor name for database>'),...)
sensors = (('b1a04f62f20a9acec3481cd90a357ae5','L1'),
           ('830fefb0f0f898515b51266033e05fa6','L2'),
           ('c486171ab3865fec2ffabe12cd24ee73','L3'),
           ('0b5cf0a60f22093fade6ae86e2b561f8','PV'))
# define your local FLM's url here - see FLM manual for details,
# for IP address query your DHCP server or use the fixed IP address you assigned
url = 'http://192.168.0.50:8080/sensor/'
# define local query (there is just one option so far - watts during minute)
query = '?version=1.0&interval=minute&unit=watt'

# connect to database
try:
    db = MySQLdb.connect(host='localhost',    # whereever you located your db
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
        # try until fetched a valid JSON result
        error = True
        while error:
            error = False
            try:
                response, content = flm.request(req, 'GET', headers=headers)
            except (httplib2.HttpLib2Error, httplib.IncompleteRead):
                error = True
            if response.status == 200:
                try:
                    data = json.loads(content)
                    logging.info(content)
                except ValueError:
                    error = True
                # it is also an error if there is no JSON content
                if data == 0:
                    error = True
            else:
                error = True
# save sensor data in database
        for (timestamp, power) in data:
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
# note - this is done in an infinite loop for now
# this may be removed to run the script via a cron job,
# but cron goes down to 1 min only...
# with the infinite loop start the script to run also after logoff from RPi:
# sudo nohup python flm_query_to_db.py &

# routine to handle errors
def handleError(e):
#    print "Error %d: %s" % (e.args[0], e.args[1])
    logging.error('Error %d: %s' % (e.args[0], e.args[1]))
    sys.exit (1)
