#!/usr/bin/python

#In parts copyright (c) 2010,2011 Roger Light <roger@atchoo.org>
#
#This Python script instantiates a MQTT client connecting to a Fluksometer
#It subscribes to the sensor topics and pushes the received sensor data into
#a mySQL database

__author__ = "Markus Gebhard, Karlsruhe"
__copyright__ = "Copyright September 2013"
__credits__ = ["raspberrypi.org", "mqtt.org", "mosquitto.org", "Simon Monk"]
__license__ = "GPL"
__version__ = "0.2"
__maintainer__ = "Markus Gebhard"
__email__ = "markus.gebhard@web.de"
__status__ = "draft"

# import MQTT access capability
import mosquitto
# import access to MySQL database and json string formatting
import MySQLdb, json
# get routines to handle date and time data
from datetime import datetime
# prepare logging to see what happens in the background
import logging, warnings
# import signal handling for external kill command and system access
import signal, sys

#global data definitions - define your sensor IDs here and provide them a name
sensors = {'b1a04f62f20a9acec3481cd90a357ae5':'L1',
           '830fefb0f0f898515b51266033e05fa6':'L2',
           'c486171ab3865fec2ffabe12cd24ee73':'L3',
           '0b5cf0a60f22093fade6ae86e2b561f8':'PV'}

#routine definitions
# routine to handle errors
def handleError(e):
    logging.error('Error %d: %s' % (e.args[0], e.args[1]))
    sys.exit (1)

# routine to properly end a job
def killHandler(signum, stackframe):
    if db.open:
        db.close()
	mqttc.disconnect()
    logging.info('Job ended')
    sys.exit(0)

#routines for MQTT handling
def on_connect(mosq, obj, rc):
    logging.info("rc: "+str(rc))

def on_message(mosq, obj, msg):
#   get sensor id from received topic post
    (na, sen, senid, typ) = msg.topic.split('/')
#	determine the sensor's measured power
    (timestamp, power, unit) = json.loads(msg.payload)	
    logging.info(sensors[senid]+' '+str(datetime.fromtimestamp(timestamp))+' '+str(power)+' '+str(unit))
#	write measurement to database
    try:
        cur.execute("""INSERT INTO flmdata (sensor, timestamp, power)
                    VALUES (%s,%s,%s)
                    ON DUPLICATE KEY UPDATE
                    timestamp = VALUES(timestamp),
                    power = VALUES(power)""",
                    (sensors[senid], str(datetime.fromtimestamp(timestamp)), power))
        db.commit()
    except MySQLdb.Error, e:
        handleError(e)

def on_publish(mosq, obj, mid):
    logging.info("mid: "+str(mid))

def on_subscribe(mosq, obj, mid, granted_qos):
    logging.info("Subscribed: "+str(mid)+" "+str(granted_qos))

def on_log(mosq, obj, level, string):
    logging.info(string)

# initialize output options
logging.basicConfig(filename='flm_query.log',
                    level = logging.ERROR, #level=logging.DEBUG,
                    filemode='a',
                    format='%(asctime)s %(message)s',
                    datefmt='%m/%d/%Y %H:%M:%S')
logging.captureWarnings(True)
# ignore warnings, e.g. that table flmdata exists...
warnings.filterwarnings('ignore')

# handle a kill signal to make an educated end on "kill <pid>"
signal.signal(signal.SIGTERM, killHandler)
# handle Ctrl-C in online mode
signal.signal(signal.SIGINT, killHandler)

# connect to database - be aware to have a database "flm" created
try:
    db = MySQLdb.connect(host='localhost', # wherever you located your db
                         user='root', # use your convenient user
                         passwd='raspberry', # and password
                         db='flm') # and database
except MySQL.Error, e:
    handleError(e)

# prepare table to write data into
# create a table to store FLM values (if it does not exist)
try:
    cur = db.cursor()
    cur.execute("""
                CREATE TABLE IF NOT EXISTS flmdata
                (
                    sensor CHAR(32),
                    timestamp TIMESTAMP,
                    power CHAR(5),
                    UNIQUE KEY (sensor, timestamp)
                )
                """)
except MySQLdb.Error, e:
    handleError(e)

# initialize MQTT client
mqttc = mosquitto.Mosquitto()
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_publish = on_publish
mqttc.on_subscribe = on_subscribe

while True:
#	connect to FLM - use your local IP address here
	mqttc.connect("192.168.0.50", 1883, 60)
#	subscribe to sensor gauges corresponding to definition
	for sensor in sensors:
		mqttc.subscribe("/sensor/"+str(sensor)+"/gauge", 0)
#	now loop for the messages subscribed to
	rc = 0
	while rc == 0:
		rc = mqttc.loop()
#	note: if there is a connection loss there should be a reconnect
#	this for now is solved with the infinite loop here