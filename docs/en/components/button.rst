Button
=======
In deep sleep mode, the button serves as a wake-up trigger for the device.

During normal operation, a short button press (less than 2.5 seconds) transitions the device into deep sleep mode. The device exits deep sleep and resumes operation upon the next button press.

In normal operation mode, a long button press initiates a system reset, after which the device boots into ``QR Reader mode``.

The button is interfaced with **GPIO21** on the ESP32-S3 microcontroller.

Hardware-based debouncing is implemented for the button, and it operates in an active-low configuration.

.. todo::
    Add test for button wake up from deep sleep.

.. include-build-file:: inc/button.inc