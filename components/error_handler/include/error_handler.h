#pragma once

typedef void (*DeinitCallback)();

void set_wifi_deinit_callback(DeinitCallback callback);
void set_mqtt_deinit_callback(DeinitCallback callback);
void set_camera_deinit_callback(DeinitCallback callback);

/**
 * @brief Restarts the device
 *
 * This function deinitializes the camera, MQTT, and Wi-Fi drivers, then
 * safely restarts the device.
 *
 * @return
 *     - Nothing as the device restarts
 */
void restart();

void deinit_components();