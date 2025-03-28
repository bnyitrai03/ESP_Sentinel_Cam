Led
====
The LED is connected to **GPIO48** on the ESP32-S3. It is used to indicate the state of the software.

.. blockdiag::

   blockdiag {
     default_fontsize = 12;
     node_width = 250;
     node_height = 80;
     span_height = 40;

     group camera_app {
       label = "Camera app";
       color = "lightblue";
       
       A [label = "1. Turned off"];
       B [label = "2. Before MQTT connection:\nLED is turned on"];
       C [label = "3. After the connection:\nBlinking with 0.5 sec period"];
     }

     group qr_code_app {
       label = "QR code reader app";
       color = "lightgreen";
       
       D [label = "1. Turned off"];
       E [label = "2. QR code not decoded yet:\nSlow blinking (1 sec)"];
       F [label = "3. QR code decoded:\nLED is turned on"];
       G [label = "4. Static config saved:\nBlink 3 times (0.5 sec)"];
     }

     error [label = "Error has occured: 0.25 sec\nfast blinking for 10 sec", 
            shape = "note", color = "red"];
   }

.. include-build-file:: inc/led.inc