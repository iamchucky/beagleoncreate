# Table of contents #

# Introduction #
Beagle on Create is a project that utilizes Beagleboard with a robotic development platform iRobot Create. The Beagleboard serves as the sensor interface as well as teleoperation device for the Create. The sensors including an USB webcam and 3 sonars. This user manual serves as a guide for users to connect to the Beagleboard and the sensors.

# Before you start #
An operational Beagle on Create setup includes following items:
  * [iRobot Create](http://store.irobot.com/shop/index.jsp?categoryId=3311368)
  * [BeagleBoard-xM](http://beagleboard.org/)
  * [Microsoft LifeCam VX-700 USB Webcam](http://www.amazon.com/Microsoft-LifeCam-VX-700/dp/B002JXKGFA)
  * Three [PING))) Ultrasonic Distance Sensor](http://www.parallax.com/tabid/768/ProductID/92/Default.aspx)
  * [WiFi Adapter](http://www.newegg.com/Product/Product.aspx?Item=N82E16833315091)
  * [iRobot USB Cable](http://store.irobot.com/product/index.jsp?productId=2818673)
  * [GPIO Power Addon board](http://beagleoncreate.googlecode.com/svn/GpioPowerAddon/GpioPowerAddon.pdf) _[gerber](http://beagleoncreate.googlecode.com/svn/GpioPowerAddon/gerber/gerber.zip)_

Please double check that the items listed above is present and is properly connected as shown in the pictures below.

![http://beagleoncreate.googlecode.com/svn/doc/images/setupfrontNew.jpg](http://beagleoncreate.googlecode.com/svn/doc/images/setupfrontNew.jpg)

![http://beagleoncreate.googlecode.com/svn/doc/images/setupbackNew.jpg](http://beagleoncreate.googlecode.com/svn/doc/images/setupbackNew.jpg)

![http://beagleoncreate.googlecode.com/svn/doc/images/setupcloseupNew.jpg](http://beagleoncreate.googlecode.com/svn/doc/images/setupcloseupNew.jpg)

# Ready for action #

Press the power button on the iRobot Create to turn on the system. Since the Beagleboard is powered from the battery of the Create, the Beagleboard will also get turned on when the Create is powered. A few seconds after the Beagleboard is powered, make sure the blue LED is lit on the Wifi adapter. This indicate that the Beagleboard has finished booting and is ready to be connected wirelessly.

## Connect through SSH ##

To be able to gain access to the Beagleboard, you first need to have SSH client such as [PuTTY](http://www.chiark.greenend.org.uk/~sgtatham/putty/download.html) to allow you SSH into the Beagleboard. In addition, you have to know the IP address that is associated with the Beagleboard you wanted to connect to. I will use "192.168.1.141" in this guide. Run the PuTTY and configure the settings as shown in the picture below. Optionally, you can save the session configurations for easier access later by clicking at save button.

![http://beagleoncreate.googlecode.com/svn/doc/images/puttysettings.png](http://beagleoncreate.googlecode.com/svn/doc/images/puttysettings.png)

Click "Open" to initiate the connection to the Beagleboard. A console will pop up to ask for username and password. Type in:
```
username: beagle
password: cornell
```

Start the beagleoncreate sensor interface program by executing following commands.
```
cd beagleoncreate
sudo ./main
```

**If you were asked for password, type in "cornell"**

Once the program started running, the PuTTY session window should look like the picture shown below.

![http://beagleoncreate.googlecode.com/svn/doc/images/sshlogin.png](http://beagleoncreate.googlecode.com/svn/doc/images/sshlogin.png)

Now you are ready to control the Beagleboard and Create using MATLAB. So **minimize** the PuTTY console, open up MATLAB program and start running your code.

**Note:** Every MATLAB connection should start with CreateBeagleInit function. For more information, see `help CreateBeagleInit` in MATLAB. For example:
```
ports = CreateBeagleInit('192.168.1.141');
```

The SSH session console should look like following picture now:

![http://beagleoncreate.googlecode.com/svn/doc/images/createbeagleinit.png](http://beagleoncreate.googlecode.com/svn/doc/images/createbeagleinit.png)



## Calibration ##
The calibration of the sensors is needed because each sonar and camera mounted position is different on every iRobot Create setup. Therefore, it is recommended to run the calibration process once before every experiment/lab.

**At this point, you should have already done all steps in the previous section.**

Simply execute following line in MATLAB to start the calibration GUI:
```
CalibGUI(ports);
```

The calibration GUI provides you a grayscale version of the video stream from the webcam on the Create as well as both ARtag pose measurement and 3 sonar distance measurements as shown in the picture below.

![http://beagleoncreate.googlecode.com/svn/doc/images/calibguiNew.png](http://beagleoncreate.googlecode.com/svn/doc/images/calibguiNew.png)

### Calibrate sonars ###
  1. Click on the **Calibrate Left/Front/Right Sonar** button to start the calibration process for of the corresponding sonar. The calibration GUI will then ask you to have the robot with the corresponding sonar facing the wall at 4 known distances.
  1. Click **Next** button when you have placed the robot at the corresponding distance.
  1. When the sonar reading is NaN, it will ask you to click Next again when the reading is good.
  1. After 4 distances are measured for the sonar, Click **Done Calibration** to finish sonar calibration. Three calibration offsets are calculated by the mean of the four measurement offsets with each sonar.
  1. The `SONAR_OFFSET` variable is automatically saved to a file named `sonar_calibration.mat` so that you can load it in the future.

### Calibrate ARtag beacon ###
The process of calibrating ARtag beacon is similar to calibrate sonars. Make sure the lighting in the room is good so that ARtag can be easily detectable.
  1. Click on the **Calibrate Camera** Button to start the calibration process. The calibration GUI will then ask you to place an ARtag in front of the camera at 4 known distances. Make sure there is only one ARtag in the camera's view.
  1. Click **Next** button when you have placed the ARtag at the corresponding distance.
  1. When there is no ARtag detected in the view, it will ask you to click Next again when there is ARtag detected.
  1. After 4 distances are measured, Click **Done Calibration** to finish beacon calibration. An calibration offset is calculated by the mean of the four measurement offsets.
  1. The `BEACON_OFFSET` variable is automatically saved to a file named `beacon_calibration.mat` so that you can load it in the future.

## Quitting the program ##
When you are done with the program, you can properly close the port and quit the `beagleoncreate` program on the Beagleboard by executing  `BeagleShutdown` function in MATLAB:
```
BeagleShutdown(ports);
```

![http://beagleoncreate.googlecode.com/svn/doc/images/beagleshutdown.png](http://beagleoncreate.googlecode.com/svn/doc/images/beagleshutdown.png)

_**This concludes the Beagle on Create User Manual.**_


---


---


# Troubleshooting #

  * **I cannot SSH into the Beagleboard.**
> There are many things that can go wrong when you can't SSH into the Beagleboard.
    1. Check that the Wifi dongle's blue LED is lit. If not, try restart the Beagleboard by toggling the power of the iRobot Create.
    1. Double check the IP address of the Beagleboard is correct when you put it into the PuTTY's configuration.

  * **Sonar reading is NaN.**
> Sonar reading is NaN when the distance reading is more than 3 meters  or when the distance reading is less than 0.

  * **Address already in use: Cannot bind/connect.**
> If you have just called `BeagleShutdown`, give it one minute and retry. Ultimately you can restart the MATLAB program.

  * **`binding: Address already in use`**
> If you tried to run `sudo ./main` and you get a message of:
```
Listener Thread: 0.
binding: Address already in use
```
> run:
```
touch main
```
> then run `sudo ./main` again.

  * **Host unreachable: connect.**
> Make sure the IP address you put into `CreateBeagleInit` is a reachable IP address.