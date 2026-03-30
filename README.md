# ESP32-DOF-Tester
WIP to use an ESP32 microcontroller with DirectOutput Framework

######################
      DESCRIPTION
######################

The intent of this project was to create an alternative to using a Teensy or pre-made pinball connection boards for interfacing with DOF for LEDs. Admittedly, I'm trying to revinvent the wheel here, but I wanted to see if I could make my own from parts I had lying around. In this case, the intent was to pass the serial data from the PC through an ESP32-S3 acting as an HID device with 16 inputs to a second board dedicated to accesories (LEDs, pluger, etc), in this case an ESP32-C3. The files here are just focused on the DOF side of things.

######################
      Status
######################

As of initial commit, the DOF testing tool loads the configuration and initially sees the microcontroller, according to the logs and a serial monitor. Unfortunately, pulsing the singular switch that is listed in the tester does nothing. The board doesn't stay connected despite numerous attempts to keep the handshake alive. I['m plagued by intermittend "EXCEPTION: Could not put the controller on com-port 'COM7' into the commandmode. Will not send data to the controller." errors. I've included simplified Cabinet.xml, directoutputconfig30.ini and GlobalConfig.B2SServer.xml files for reference, as well as the debug log from DOF tester.


--QUICK NOTE--
Full disclosure that I relied quite heavily on Google Gemini, ChatGPT and Claude Code for assistance in understanding the logs and writing the code. I'm not proud of it, but at the same time, I could not have gotten far without it.
