I2C Manager
============
The ``I2C Manager`` implementation uses the new `ESP-IDF I2C master driver <https://docs.espressif.com/projects/esp-idf/en/v5.4.1/esp32s3/api-reference/peripherals/i2c.html#inter-integrated-circuit-i2c>`_, 
which was introduced in ESP-IDF v5.0.

To the I2C bus the following devices are connected:

- `BQ25622 <https://www.ti.com/product/BQ25622>`_ battery charger

- `OPT3005 <https://www.ti.com/product/OPT3005>`_ ambient light sensor

The I2C pins are connected to the ESP32-S3 as follows:

====== ========
I2C    ESP32-S3
====== ========
SDA    GPIO 4
SCL    GPIO 5
====== ========

.. include-build-file:: inc/i2c_manager.inc