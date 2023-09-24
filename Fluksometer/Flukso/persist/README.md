**persist_mqtt.js** is a .js script running on node.js to persist Fluksometer
readings in a MySQL database - so it needs [node.js](http://nodejs.org) installed...<br/>
It connects to the FLM's MQTT broker on the discovered IP address(es)
using the multicast DNS service discovery - so there is no further
configuration to change.

Make sure the **mdns, mqtt** and **mysql** node_modules are installed; if they are not installed, install them using either

    npm install mdns mqtt mysql
    
or just

    npm install
    
utilizing the provided [package.json](package.json) file.

Note: To get the mdns module installed on a Raspberry Pi by npm you also need the
package libavahi-compat-libdnssd-dev installed in the system:

    sudo apt-get update
    sudo apt-get install libavahi-compat-libdnssd-dev

As mdns uses a compatibility layer, be aware that it throws warnings on use; follow
the links to get an understanding what has happened (and then ignore them). 

#Starting the script

After start (simply use **./persist.sh**), the script checks if the persistence
table in the database FLM exists; if not, it creates the table together with a table for sensor configuration.<br/>
Subscribing to the sensor topics, the script starts to insert received
values (i.e. mqtt messages on topic /sensor/#) in the database for later
retrieval, e.g. by a charting application.

#Precondition: Setting up the database
Make sure you have installed a database; in my case it is MySQL.

    sudo apt-get install mysql-server
    
This installs the database and all dependent packages not already installed.
During installation you are asked for a root password of the database; I chose
'raspberry' for this (as a no-brainer to the default RasPi password)
Log into the database to create the schema used for storing Flukso data:

    mysql -u root -p
    mysql> create database flm;
    mysql> use flm;

By this you log into the database as root; provide above given password when asked.
Create the database used in the logging scripts; chose whatever you like, but
be sure to have the same database applied also in the scripts. With the use
command you change to the created database for further actions.

To use the mySQL database for storing data not only via the root user
you have to set up a corresponding "other" user; this can be done from
the mySQL command line by following commands (assuming you are still logged
on to the database - if not, use

    mysql -u root -p flm 
 
from the prompt):

    mysql> create user 'pi'@'localhost' identified by 'raspberry';
    mysql> grant all privileges on flm.* to 'pi'@'localhost';
    mysql> flush privileges;
    mysql> set password for 'pi'@'localhost' = password('raspberry');
    mysql> quit

All this for convenience can also be achieved by just running the [createdb.sh](createdb.sh) script with parameters `flm pi raspberry`.

Now you may log on to the database also as user 'pi':
    
    pi@raspberry ~ $ mysql -u pi -p flm
    mysql> show databases;
    mysql> show tables;
    mysql> select count(*) from flmdata;

Markus Gebhard, (c) 2014-2016, all code under MIT license without any warrenty.

Have fun...
