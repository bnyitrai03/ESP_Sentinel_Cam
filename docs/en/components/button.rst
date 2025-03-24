Button
=======
When the device is in deep sleep mode, the button can be used to wake up the device.

When the device is operating normally, a short button press (less than 2.5 seconds) will put the device in deep sleep mode. The device will wake up when the button is pressed again.

In the normal operation mode, a long button press will reset the device. Then the device will boot in ``QR Reader mode``.

The button is connected to the ESP32-S3 as follows:
::
    Button  ESP32-S3
    ------  --------
    Button  GPIO 21

The button is debounced in hardware and active low.

.. todo::
    Add test for button wake up from deep sleep.

.. include-build-file:: inc/button.inc