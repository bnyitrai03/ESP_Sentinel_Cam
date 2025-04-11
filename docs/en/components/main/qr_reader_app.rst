QR Code Reader App
===================

The ``QR Reader App`` is responsible for scanning a QR code, extracting WiFi and server information, connecting to the network, and retrieving a ``static configuration`` from the ``config server``. 
Once the configuration is obtained, the device saves it and restarts to initiate the ``Camera mode``.

Components
-------------

The ``QR Reader App`` follows a singleton design pattern and operates using FreeRTOS tasks. Below are the main components:

- **QRReaderApp**: Main class managing the application lifecycle

- **Camera**: Captures frames for QR code decoding

- **QRDecoder**: Processes images to decode the QR code

- **WiFi Module**: Handles network connection

- **HTTP Client**: Fetches the ``static configuration`` from the ``config server``

- **Storage**: Saves the retrieved configuration

Flowchart
----------

The following flowchart represents the main logic of the application:

.. figure:: ../../../_static/qr_app_flowchart.png
        :align: center
        :scale: 80%
        :alt: Image missing

        Flowchart depicting the ``QR Reader App``

Error Handling
---------------

- If QR decoding fails, the process retries until a valid QR code is found.

- If WiFi connection fails, the system retries multiple times before restarting.

- If the ``config server`` GET request fails, the application retries a set number of times before restarting.

External Dependencies
----------------------

- `ArduinoJson <https://github.com/bblanchon/ArduinoJson>`_

- `quirc <https://github.com/dlbeer/quirc>`_

- `esp32-camera <https://github.com/espressif/esp32-camera>`_

.. include-build-file:: inc/qr_reader_app.inc