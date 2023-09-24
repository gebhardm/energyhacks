# Receive local tmpo messages

With firmware version 2.4.4 onwards the [Fluksometer](http://flukso.net) contains a "tmpo daemon" that stores counter data of the sensors locally and publishes them to the flukso-API; as mqtt-messages, these can be received and looked at.

The FLM publishes tmpo messages on topic 

     /sensor/<sensor id>/tmpo/<rid>/<level>/<block id>
    
**rid** is some release id (seems to be always 0), **level** is the size of the data package published (2^8 to 2^20 time-value-pairs), and **block id** the corresponding timestamp indicating the start time of the block contained data. Block data is compressed using gzip, thus requires to be decompressed for computation.

The [tmpo_getter.js](tmpo_getter.js) script is a quick and dirty hack to obtain tmpo data published by the own fluksometer - it uses the **mdns** detection and subscribes to the above mentioned tmpo topics.

It outputs the received messages and their decompressed payload, which looks as follows:

	/sensor/<sid>/tmpo/0/8/1422801920/gz
	{ h: 
	   { cfg: 
	      { id: '<sid>',
	        class: 'analog',
	        type: 'electricity',
	        port: [Object],
	        function: '<sensor name>',
	        rid: 0,
	        current: 50,
	        data_type: 'counter',
	        voltage: 230 },
	     tail: [ 1422802131, 637842 ],
	     vsn: 1,
	     head: [ 1422801984, 637840 ] },
	  t: [ 0, 66, 81 ],
	  v: [ 0, 1, 1 ] }

These blocks are stored on the FLM in `/usr/share/tmpo/sensor` and can potentially also be accessed directly from the FLM itself - which is an open request to enhance the tmpo daemon to publish queried data blocks.

**Note**: Data blocks contain counter data only - values **v:** deliver the delta respective increase in Wh with the corresponding time **t:** delta respective interval; thus "gauges" need to be approximated if desired.

Idea is to send a message on topic `/sensor/sid/query` with payload the query time interval and receive a corresponding /query/sid response containing the requested data blocks. 
