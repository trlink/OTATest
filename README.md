# OTATest
Public repository for installing TR-Link

#What's TR-Link?
TR-Link was designed to send sensor data (CO2, O2, CH4) out of an abandoned mine, where we wanted to dive... 
We knew that this dive is only possible when the CO2 level in this mine is verry low, so we needed to monitor the level without the need to enter the mine with a gas analyser...

That was the time where TR-Link was born... 

One and a half year later it became a mesh-network communication device for my other hobby Ham-Radio:

You can  

* send Messages to other users (and get status information aboud transferred and read), with a limit of 150 chars
* send Chat messages to a group, with a limit of 150 chars
* Broadcast your GPS position
* Use the device as Packet-Radio-Modem with linpac (linux) or FlexNet (windows) (MTU: 250)
* Use it as a Data-Modem (Hayes-compatible, MTU: 250)
* Use it as pass-through USB serial modem (MTU: 250)
* Send and receive emails (when linked over our server), with a limit of 150 chars 

The User-Interface is a nice Web-GUI (Web-Site) which can be viewed with any modern browser (Chrome, Edge, ...)

To build your Network, you only need to lay out TR-Link devices, the devices build the mesh-network by itself. You only need to enter a Node-Name and a Device ID - but every config option is described on the web-page...
The more nodes, the larger is the area which is covered...
If you want to cover huge areas, you can connect them over the internet. We also have a server which allows to interconnect the devices (where no static ip is available). 

#Install

The installation-process is fairly simple: get an ESP32 with LoRa-Modem or a TTGO T-Beam, download this sketch, enter credentials for a WLan and remove the comment for your Hardware:

If you have a T-Beam, uncomment the define HARDWARE_TBEAM, if you have an ESP, then for HARDWARE_ESP...

Now the sketch formats the SPIFFS off your device, downloads the Web-Site und the binary program and install it on your device, when finished, it reboots the MCU and you are ready to go:

Now connect to the TR-Link Access-Point and browse to 192.168.5.1 and configure the device - when configurred propperly, it would connect to an existing network or waits until another node can be reached...

Other ressources:

https://teamrosengarten.weebly.com/tr-link.html (German)<br/>
https://www.facebook.com/TeamRosengartenLink (German, sometimes english)
