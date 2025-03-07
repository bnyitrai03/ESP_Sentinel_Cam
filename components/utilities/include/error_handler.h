#pragma once

#include <cstdint>

typedef void (*DeinitCallback)();

/**
 * @brief Sets the callback function for WiFi deinitialization
 *
 * @param callback Function to be called for WiFi deinitialization
 */
void set_wifi_deinit_callback(DeinitCallback callback);

/**
 * @brief Sets the callback function for MQTT deinitialization
 *
 * @param callback Function to be called for MQTT deinitialization
 */
void set_mqtt_deinit_callback(DeinitCallback callback);

/**
 * @brief Sets the callback function for camera deinitialization
 *
 * @param callback Function to be called for camera deinitialization
 */
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

/**
 * @brief Deinitializes all components
 *
 * This function deinitializes the camera, MQTT, and Wi-Fi drivers.
 */
void deinit_components();

/**
 * @brief This function increases and returns the error count stored in the NVS.
 *
 * @return The error count
 */
uint32_t get_error_count();