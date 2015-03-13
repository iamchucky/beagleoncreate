# Table of contents #


# Hardware #
## Components ##
  * [iRobot Create](http://store.irobot.com/shop/index.jsp?categoryId=3311368)
  * [BeagleBoard-xM](http://beagleboard.org/)
  * [Microsoft LifeCam VX-700 USB Webcam](http://www.amazon.com/Microsoft-LifeCam-VX-700/dp/B002JXKGFA)
  * Three [PING))) Ultrasonic Distance Sensor](http://www.parallax.com/tabid/768/ProductID/92/Default.aspx)
  * [WiFi Adapter](http://www.newegg.com/Product/Product.aspx?Item=N82E16833315091)
  * [iRobot USB Cable](http://store.irobot.com/product/index.jsp?productId=2818673)
  * [GPIO Power Addon board](http://beagleoncreate.googlecode.com/svn/GpioPowerAddon/GpioPowerAddon.pdf) _[gerber](http://beagleoncreate.googlecode.com/svn/GpioPowerAddon/gerber/gerber.zip)_

## High-Level Block diagram ##
![http://beagleoncreate.googlecode.com/svn/doc/images/hardwaretoplevel.png](http://beagleoncreate.googlecode.com/svn/doc/images/hardwaretoplevel.png)

## Power ##
The [iRobot Create's Cargo Bay Connector](http://www.irobot.com/filelibrary/pdfs/hrd/create/Create%20Open%20Interface_v2.pdf) provide battery ground and voltage in order to power additional sensors and peripherals. We utilized the Create's battery voltage (~16V) to power the [BeagleBoard-xM](http://beagleboard.org/) (5V). The [GPIO Power Addon board](http://beagleoncreate.googlecode.com/svn/GpioPowerAddon/GpioPowerAddon.pdf) has voltage converter to convert 16V power down to both 5V and 12V. In this case, we grab pin 10 (Switched Vpwr) and pin 21 (GND) from the Create Cargo Bay Connector and feed it into [GPIO Power Addon board](http://beagleoncreate.googlecode.com/svn/GpioPowerAddon/GpioPowerAddon.pdf) to produce 5V power for the [BeagleBoard-xM](http://beagleboard.org/).

![http://beagleoncreate.googlecode.com/svn/doc/images/powerblockdiagram.png](http://beagleoncreate.googlecode.com/svn/doc/images/powerblockdiagram.png)

## Sensors ##
The USB webcam can be plugged into the [BeagleBoard-xM](http://beagleboard.org/)'s USB port and doesn't require extra power source. The three [PING))) Ultrasonic Distance Sensors](http://www.parallax.com/tabid/768/ProductID/92/Default.aspx) however requires 5V signals in order to perform the measurement, the details can be found [here](http://www.parallax.com/Portals/0/Downloads/docs/prod/acc/28015-PING-v1.6.pdf). Therefore, we utilized the the GPIO pins from the expansion connectors on the [BeagleBoard-xM](http://beagleboard.org/) through a voltage level translator IC [TXB0108](http://www.ti.com/lit/ds/symlink/txb0108.pdf) from TI, since the GPIO pin voltage level from the [BeagleBoard-xM](http://beagleboard.org/) is 1.8V and the sonar requires 5V signals. To see the detail connections regarding the voltage level shifter, see the schematic for [GpioPowerAddon](http://beagleoncreate.googlecode.com/svn/GpioPowerAddon/GpioPowerAddon.pdf).
# Software #
## Components ##
  * Create
> > All the iRobot Create related message passing. The purpose of this class is to listen for the TCP messages ([Open Interface Protocol](http://www.irobot.com/filelibrary/pdfs/hrd/create/Create%20Open%20Interface_v2.pdf)) from [MatlabToolboxiRobotCreate](http://code.google.com/p/beagleoncreate/source/browse/#svn%2FMatlabToolboxiRobotCreate) at the remote host and pass the message through serial to the iRobot Create. When a message is sent from the iRobot Create, it is sent through wireless TCP protocol back to the remote host.

  * Camera
> > The Camera class utilizes the [gstreamer](http://gstreamer.freedesktop.org/) library to access images from the webcam attached. The image from the webcam is then process through ARtagLocalizer class to detect and estimate the pose of any ARtags in view. A beacon message is sent through wireless UDP protocol on every camera frame.

  * Sonar
> > The Sonar class utilizes the [sysfs](http://en.wikipedia.org/wiki/Sysfs) for gpio pins on the BeagleBoard. The Sonar class sets the corresponding gpio pin for the sonar as output, then toggles the gpio pin high and low in order to send pulses to initiate every measurement. Thereafter, the gpio pin is set as input to enable an edge changing trigger. A time of flight is measured and calculated into distance measurement.

  * Control
> > This class is for any control to the BeagleBoard. It currently only allows the controls of the universal gripper and the video streaming on/off control.

  * main
> > The main function that tie all classes together and spawn threads accordingly. At the beginning of main function, a listening-to-message thread (ListenMessage) is created and allow any incoming TCP connection to initiate and parse packets accordingly (ProcessPackets). Once an INIT message is received, a sensor thread is created (MakeConnection) and allow more threads (Camera/Sonar/Create) to be created. The process is then entered into normal operation until an END message is received.