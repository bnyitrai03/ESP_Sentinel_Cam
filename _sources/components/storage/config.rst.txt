Config
=======
The device operates using two types of configurations: a ``static`` and a ``dynamic`` configuration.

Static Configuration
--------------------
The ``static configuration`` is obtained through a GET request to the ``config server``.

An example of the ``static configuration`` is as follows:

.. code-block:: json

   {
       "uuid": "1ddffcf8-a60c-489b-860e-1c4cb13048c1",
       "mqttAddress": "mqtt://00.111.22.33:1234",
       "mqttUser": "testuser",
       "mqttPassword": "123456",
       "imageTopic": "/cam1/image",
       "imageAckTopic": "/cam1/image_ack",
       "healthReportTopic": "/cam1/health",
       "healthReportRespTopic": "/cam1/health_resp",
       "logTopic": "/cam1/log",
       "cameraMode": "GRAY"
   }

Where:

- ``uuid``: Unique identifier for the device (read from the QR code)
- ``mqttAddress``: MQTT broker connection url
- ``mqttUser``: Username for MQTT authentication
- ``mqttPassword``: Password for MQTT authentication
- ``imageTopic``: MQTT topic for image transmission
- ``imageAckTopic``: MQTT topic for image header acknowledgment
- ``healthReportTopic``: MQTT topic for sending the sensor data
- ``healthReportRespTopic``: MQTT topic for receiving the ``dynamic config`` acknowledgment
- ``logTopic``: MQTT topic for logging
- ``cameraMode``: Camera operating mode (**GRAY** or **COLOR**)

Dynamic Configuration
---------------------
The ``dynamic configuration`` is received through the ``healthReportRespTopic``.

An example of the ``dynamic configuration`` is as follows:

.. code-block:: json

   {
        "configId": "8D8AC610-566D-4EF0-9C22-186B",
        "timing": [
            {
                "period": -1,
                "start": "00:00:00",
                "end": "07:00:00"
            },
            {
                "period": 30,
                "start": "07:00:00",
                "end": "12:00:00"
            },
            {
                "period": 40,
                "start": "12:00:00",
                "end": "15:00:00"
            },
            {
                "period": 30,
                "start": "15:00:00",
                "end": "17:00:00"
            },
            {
                "period": 40,
                "start": "17:00:00",
                "end": "22:00:00"
            },
            {
                "period": -1,
                "start": "22:00:00",
                "end": "23:59:59"
            }
        ]
    }

Where:

- ``configId``: Unique identifier for the configuration
- ``timing``: Array of working hours with the following keys:

  - ``period``: Period of the image capture in seconds. If the value is **-1**, the camera is turned off.

  - ``start``: Start time for the period in `HH:MM:SS` format

  - ``end``: End time for the period in `HH:MM:SS` format

.. include-build-file:: inc/config.inc