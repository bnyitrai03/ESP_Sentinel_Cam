CPU Temperature Sensor
=======================
The ``CpuTemp`` class provides an interface for interacting with the ESP32's internal CPU `temperature sensor <https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/temp_sensor.html>`_.
It implements the ``ISensor`` interface and supports initialization, reading, and retrieving the CPU temperature in degrees Celsius.

.. include-build-file:: inc/cpu_temp.inc