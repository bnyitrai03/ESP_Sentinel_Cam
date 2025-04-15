Led
====
The RGB LED is connected to the ESP32-S3 with the following pin configuration:

===== ==========
LED   ESP32-S3
===== ==========  
Red   GPIO48
Green GPIO38
Blue  GPIO39
===== ==========

It is used to indicate the state of the software with different colors and blinking patterns.

.. blockdiag::
   blockdiag {
     default_fontsize = 12;
     node_width = 250;
     node_height = 80;
     span_height = 40;

     group camera_app {
       label = "Camera App";
       color = "lightblue";

       A [label = "1. Turned off"];
       B [label = "2. Before MQTT connection:\nLED is turned on (white)"];
       C [label = "3. After the connection:\nOrange"];
     }

     group qr_code_app {
       label = "QR Code Reader App";
       color = "lightgreen";

       D [label = "1. Turned off"];
       E [label = "2. QR code not decoded yet:\nBlue"];
       F [label = "3. QR code decoded:\nLED is turned on (white)"];
       G [label = "4. Static config saved:\nGreen (0.5 sec)"];
     }

     error [label = "Error occurred:\nRed fast blinking (0.25 sec)",
            shape = "note", color = "red"];
   }

LED Patterns
------------

The RGB LED class supports the following predefined patterns:

- **OFF**: LED is turned off (no color)
- **ON**: White color, constant
- **NO_QR_CODE**: Blue color, constant – indicates QR code has not been detected yet
- **STATIC_CONFIG_SAVED**: Green color, constant – indicates static configuration has been saved
- **MQTT_CONNECTED**: Orange color, constant – indicates successful MQTT connection
- **ERROR_BLINK**: Red blinking with 0.25 second interval – indicates an error has occurred

Hardware Implementation
-----------------------

The RGB LED is implemented using PWM control via the ESP32's `LEDC <https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/ledc.html>`_ peripheral.  
Each color channel (Red, Green, Blue) is connected to a separate GPIO pin and controlled with 8-bit PWM resolution at a 5kHz frequency.

.. include-build-file:: inc/rgb_led.inc