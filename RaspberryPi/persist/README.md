# Persist FLM data
*persist_mqtt.js* is a .js script running on node.js to persist Fluksometer
readings in a MySQL database. It connects to the FLM's MQTT broker on the
specified IP address (change it to your FLM's or broker's address!)
After start, the script checks if the persistence table in the database FLM
exists; if not, it creates the table.
Subscribing to the sensor topics, the script starts to insert received
values in the database for later retrieval, e.g. by the chart.


#Using mySQL to store FLM data

To use the mySQL database for storing data not only via the root user
you have to set up a corresponding "other" user; this can be done from
the mySQL command line by following commands:

Log into the mySQL database called 'flm':
> pi@raspberrypi ~ $ mysql -u root -p flm

> mysql> create user 'pi'@'localhost' identified by 'raspberry';<br/>
> mysql> grant all privileges on flm.* to 'pi'@'localhost';<br/>
> mysql> flush privileges;<br/>
> mysql> set password for 'pi'@'localhost' = password('raspberry');<br/>
> mysql> quit<br/>

Now you may log on to the database also as user 'pi':
> pi@raspberry ~ $ mysql -u pi -p flm<br/>

> mysql> show databases;<br/>
> mysql> show tables;<br/>
> mysql> select count(*) from flmdata;<br/>

Note: If the database flm was not created beforehand, make so by 
> mysql> create database flm;<br/>
> mysql> use flm;<br/>

Markus Gebhard, May 2014, all code under MIT license without any warrenty.

Have fun...