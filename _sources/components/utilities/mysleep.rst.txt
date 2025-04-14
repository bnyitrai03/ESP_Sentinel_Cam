Sleep
=====
This component provides simple sleep functions to reduce power consumption by putting the device into ``deep sleep`` mode.
The device can wake up based on the following triggers:

- The start of the next **period**
- The next **timing**
- A button press

Current Draw in Normal Operation Mode
--------------------------------------

The table below shows the current drawn by the device at different battery voltage levels during normal operation:

=============== ==============
Battery Voltage Current Drawn
=============== ==============
4.2 V           400 mA
4.0 V           420 mA
3.8 V           460 mA
3.6 V           500 mA
3.3 V           560 mA

=============== ==============

.. note:: 
   During ``deep sleep``, the current draw is less than **10 mA**, significantly reducing power consumption.

.. include-build-file:: inc/mysleep.inc