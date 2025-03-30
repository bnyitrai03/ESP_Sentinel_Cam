Camera App
===========
The ``Camera App`` is responsible for running the image capturing application on the ESP32-S3 device. It handles:

- Initializing all necessary components (WiFi, MQTT, sensors, camera)

- Managing configuration updates

- Capturing and sending images to an MQTT broker

- Health reporting

Components
-----------

- **Camera**: Handles image capture, also manages the camera hardware initialization and image buffer storage.

- **WiFi**: Manages network connection and time synchronization. Handles connection establishment, maintenance, and automatic reconnection logic.

- **MQTT**: Facilitates communication with the MQTT broker for health reports, images, and configuration updates. Implements topic management and message acknowledgment.

- **Config**:  Stores and manages the ``dynamic configuration``, handles it's loading, validation and update.

- **Sensors**:  Interfaces with sensors to collect data like battery temperature and charge. Formats sensor readings for inclusion in health reports.

Flowchart
----------

The following flowchart represents the main logic of the application:

.. figure:: ../../../_static/camera_app_flowchart.png
        :align: center
        :alt: Flowchart depicting the ``Camera App``

        Flowchart depicting the ``Camera App``

External Dependencies
----------------------

- `ArduinoJson <https://github.com/bblanchon/ArduinoJson>`_

- `esp32-camera <https://github.com/espressif/esp32-camera>`_

.. include-build-file:: inc/camera_app.inc