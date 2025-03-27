QR Code Decoder
================
The QR Code Decoder component is used to decode QR codes from images. It is based on the `quirc <https://github.com/dlbeer/quirc>`_ library.

The QR code format is the following: ::

    WIFI_SSID|WIFI_PASSWORD|CONFIG_SERVER_URL

Where:

- **WIFI_SSID**: The name of the Wi-Fi network.

- **WIFI_PASSWORD**: The password of the Wi-Fi network.

- **CONFIG_SERVER_URL**: The URL of the configuration server.

After decoding these will be saved in the **NVS** storage.

.. include-build-file:: inc/qr_decoder.inc